// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/BrawlProjectile_Explosive.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BrawlPoolSubsystem.h"

void ABrawlProjectile_Explosive::OnActivate()
{
	Super::OnActivate();
	bHasExploded = false;
}

void ABrawlProjectile_Explosive::OnDeactivate()
{
	Super::OnDeactivate();
}

void ABrawlProjectile_Explosive::GetPrewarmRequirements(TMap<TSubclassOf<AActor>, int32>& OutRequirements, int32 BaseCount) const
{
	if (SplinterClass)
	{
		OutRequirements.FindOrAdd(SplinterClass) += SplinterCount * BaseCount;
	}
}

void ABrawlProjectile_Explosive::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 이미 폭발했으면 무시
	if (bHasExploded) return;
	
	// 발사자(Instigator)는 무시
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator() 
		|| OtherActor == this || OtherActor->IsA(ABrawlProjectile::StaticClass())) return;

	// 폭발 처리 (부모의 OnHit에서 Deactivate될 수 있으므로 먼저 처리)
	Explode(Hit);

	// 부모의 OnHit 실행 (데미지 처리 및 Deactivate 호출)
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

void ABrawlProjectile_Explosive::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 이미 폭발했으면 무시
	if (bHasExploded) return;

	// 발사자(Instigator)는 무시
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()
		|| OtherActor == this || OtherActor->IsA(ABrawlProjectile::StaticClass()))
		return;

	// 폭발 처리를 위한 더미 HitResult 생성 (또는 SweepResult 활용)
	FHitResult Hit = SweepResult;
	if (Hit.ImpactPoint.IsZero())
	{
		Hit.ImpactPoint = GetActorLocation();
		Hit.Normal = (GetActorRotation().Vector() * -1.0f);
	}

	// 폭발 처리
	Explode(Hit);

	// 부모의 OnBeginOverlap 실행 (데미지 처리 및 Deactivate 호출)
	Super::OnBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void ABrawlProjectile_Explosive::OnLifeTimeExpired()
{
	// 수명이 다해서 죽는 경우 (OnHit을 거치지 않음)
	if (!bHasExploded)
	{
		// 허공에서 폭발
		FHitResult DummyHit;
		DummyHit.Location = GetActorLocation();
		DummyHit.Normal = FVector::UpVector; // 기본 위쪽
		Explode(DummyHit);
	}

	Super::OnLifeTimeExpired();
}

void ABrawlProjectile_Explosive::Explode(const FHitResult& HitResult)
{
	// 월드가 유효한지 검사
	if (!GetWorld() || !GetWorld()->IsGameWorld()) return;
	
	// 이미 폭발한 경우 종료
	if (bHasExploded) return;
	bHasExploded = true;
	
	// 실제 충돌 지점(표면)을 사용하도록 개선. 정보가 없으면 중심점 사용.
	FVector BaseLocation = HitResult.ImpactPoint.IsZero() ? HitResult.Location : HitResult.ImpactPoint;

	// 폭발 시각/청각 효과 재생 (모든 클라이언트)
	PlayHitEffects(BaseLocation, HitResult.Normal, HitResult.GetActor());

	// 데미지 및 자탄 생성은 서버에서만 수행
	if (HasAuthority())
	{
		// 폭발 범위 데미지 처리
		ExplodeDamage(BaseLocation);

		// 파편 생성
		SpawnSplinters(BaseLocation, HitResult.Normal);
	}
}

void ABrawlProjectile_Explosive::ExplodeDamage(const FVector& Location)
{
	// 범위 데미지 설정이 되어 있지 않으면 수행하지 않음 (에러가 아님)
	if (ExplosionRadius <= 0.0f || ExplosionRadialDamage <= 0.0f)
	{
		return;
	}
	
	FVector ExplosionLocation = Location;
	TArray<FHitResult> OverlapResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
	if (GetInstigator()) QueryParams.AddIgnoredActor(GetInstigator());

	// 범위 내의 모든 대상 감지
	bool bHasOverlap = GetWorld()->SweepMultiByChannel(
		OverlapResults,
		ExplosionLocation,
		ExplosionLocation + FVector(0, 0, 1), // 아주 미세한 이동으로 스윕 유도
		FQuat::Identity,
		ECC_Pawn, // 캐릭터 위주로 감지
		SphereShape,
		QueryParams
	);
	
	if (bHasOverlap)
	{
		TArray<AActor*> DamagedActors;
		for (const FHitResult& Overlap : OverlapResults)
		{
			AActor* Victim = Overlap.GetActor();
			if (Victim && !DamagedActors.Contains(Victim))
			{
				DamagedActors.Add(Victim);
					
				// GAS 데미지 적용
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Victim);
				if (TargetASC && DamageSpecHandle.IsValid())
				{
					FGameplayEffectSpec* OriginalSpec = DamageSpecHandle.Data.Get();
					// Instigator(공격자)의 ASC가 필요함 (Spec 생성 주체)
					UAbilitySystemComponent* InstigatorASC = OriginalSpec->GetContext().GetInstigatorAbilitySystemComponent();
					if (!InstigatorASC) InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

					if (InstigatorASC && OriginalSpec->Def)
					{
						// 새 Spec 생성 (Attack Damage)
						FGameplayEffectContextHandle Context = OriginalSpec->GetContext();
						FGameplayEffectSpecHandle RadialHandle = InstigatorASC->MakeOutgoingSpec(
							OriginalSpec->Def->GetClass(), OriginalSpec->GetLevel(), Context);

						if (RadialHandle.IsValid())
						{
							// 만약 ExplosionRadialDamage가 설정되어 있다면 그 값으로 덮어쓰기
							if (ExplosionRadialDamage > 0.0f)
							{
								static FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
								RadialHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, ExplosionRadialDamage);
							}
							
							// 타겟에게 적용
							TargetASC->ApplyGameplayEffectSpecToSelf(*RadialHandle.Data.Get());
						}
					}
				}
			}
		}
	}

	// 디버그 시각화 (개발용)
	// DrawDebugSphere(GetWorld(), ExplosionLocation, ExplosionRadius, 12, 
	// 	FColor::Orange, false, 2.0f);
}

void ABrawlProjectile_Explosive::SpawnSplinters(const FVector& Location, const FVector& Normal)
{
	if (!SplinterClass || SplinterCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnSplinters Failed: Invalid SplinterClass or SplinterCount == 0"))
		return;
	}

	// 파편 확산 로직 (방사형: Radial)
	// 바닥(Normal) 기준이 아니라, 게임 특성상 월드 Z축 기준 평면 확산이 자연스러움
	FVector UpVector = FVector::UpVector; 
	FVector ForwardVector = GetActorForwardVector();
	
	// 만약 벽에 맞았을 때 튕겨나가는 느낌을 주려면 Normal을 반영해야 하지만,
	// 스파이크의 경우 보통 터진 자리에서 6방향으로 퍼짐.
	// 여기서는 단순히 월드 기준 6방향(Hexagon)으로 퍼지게 구현.
	float AngleStep = 360.0f / (float)SplinterCount;
	UBrawlPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UBrawlPoolSubsystem>();

	for (int32 i = 0; i < SplinterCount; i++)
	{
		float CurrentYaw = i * AngleStep;
		FRotator SplinterRot = FRotator(0, CurrentYaw, 0);
		
		// 지면 바로 위에서 생성되도록 오프셋 최소화 (파묻힘 방지용)
		FVector SpawnLocation = Location + (FVector::UpVector * 2.0f);
		FTransform SpawnTransform(SplinterRot, SpawnLocation);

		// 주입할 데미지 스펙 핸들 생성 로직
		FGameplayEffectSpecHandle NewHandle;
		if (DamageSpecHandle.IsValid())
		{
			FGameplayEffectSpec* OriginalSpec = DamageSpecHandle.Data.Get();
			UAbilitySystemComponent* InstigatorASC = OriginalSpec->GetContext().GetInstigatorAbilitySystemComponent();
			if (!InstigatorASC) InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

			if (InstigatorASC && OriginalSpec->Def)
			{
				FGameplayEffectContextHandle Context = OriginalSpec->GetContext();
				NewHandle = InstigatorASC->MakeOutgoingSpec(OriginalSpec->Def->GetClass(), OriginalSpec->GetLevel(), Context);
				if (NewHandle.IsValid())
				{
					static FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
					float OriginalDamage = OriginalSpec->GetSetByCallerMagnitude(DamageTag, false, -1.0f);
					if (OriginalDamage > 0.0f)
					{
						NewHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, OriginalDamage * SplinterDamageScale);
					}
				}
			}
		}

		AActor* SpawnedActor = nullptr;
		if (PoolSubsystem)
		{
			// 콜백을 통해 OnActivate 이전에 데미지 초기화 수행
			SpawnedActor = PoolSubsystem->GetFromPool(SplinterClass, SpawnTransform, GetOwner(), GetInstigator(), 
				[&NewHandle](AActor* InActor)
				{
					if (ABrawlProjectile* Splinter = Cast<ABrawlProjectile>(InActor))
					{
						Splinter->InitializeProjectile(NewHandle);
					}
				});
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnedActor = GetWorld()->SpawnActor<AActor>(SplinterClass, SpawnTransform, SpawnParams);
			
			if (ABrawlProjectile* Splinter = Cast<ABrawlProjectile>(SpawnedActor))
			{
				Splinter->InitializeProjectile(NewHandle);
			}
		}
	}
}
