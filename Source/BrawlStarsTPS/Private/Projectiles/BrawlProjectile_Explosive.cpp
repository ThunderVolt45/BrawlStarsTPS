// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/BrawlProjectile_Explosive.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void ABrawlProjectile_Explosive::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 이미 폭발했으면 무시
	if (bHasExploded) return;

	// 부모의 OnHit 실행 (데미지 처리 등)
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	// 폭발 처리
	Explode(Hit);
	
	// 폭발 후 파괴 (부모 OnHit에서도 파괴할 수 있지만, 확실히 하기 위해)
	// 하지만 부모 OnHit에서 Destroy()를 호출하면 Destroyed()가 불리고, 거기서 중복 Explode 될 수 있으므로
	// bHasExploded 플래그가 중요함.
}

void ABrawlProjectile_Explosive::Destroyed()
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

	Super::Destroyed();
}

void ABrawlProjectile_Explosive::Explode(const FHitResult& HitResult)
{
	// 월드가 유효한지 검사
	if (!GetWorld()) return;
	
	// 현재 월드가 "게임 월드" 가 아니라면 (즉, 에디터 프리뷰라면) 종료
	if (!GetWorld()->IsGameWorld()) return;
	
	// 이미 폭발한 경우 종료
	if (bHasExploded) return;
	bHasExploded = true;

	// 폭발 효과 (VFX/SFX) - 필요 시 여기에 추가 (GameplayCue 권장)

	// 파편 생성
	SpawnSplinters(HitResult.Location, HitResult.Normal);
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
	FRotator BaseRot = FRotator(0, 0, 0); // 월드 0도 기준

	for (int32 i = 0; i < SplinterCount; i++)
	{
		float CurrentYaw = i * AngleStep;
		FRotator SplinterRot = FRotator(0, CurrentYaw, 0);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = GetInstigator();
		
		// 충돌 방지를 위해 약간 띄움
		FVector SpawnLocation = Location + (FVector::UpVector * 20.0f); 

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SplinterClass, SpawnLocation, SplinterRot, SpawnParams);
		if (ABrawlProjectile* Splinter = Cast<ABrawlProjectile>(SpawnedActor))
		{
			// 데미지 Spec 복제 및 수정
			if (DamageSpecHandle.IsValid())
			{
				// 기존 Spec 복사
				FGameplayEffectSpec* OriginalSpec = DamageSpecHandle.Data.Get();
				FGameplayEffectSpec* NewSpec = new FGameplayEffectSpec(*OriginalSpec);
				
				// 데미지 스케일 적용
				static FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
				float OriginalDamage = NewSpec->GetSetByCallerMagnitude(DamageTag, false, -1.0f);
				
				if (OriginalDamage > 0.0f)
				{
					NewSpec->SetSetByCallerMagnitude(DamageTag, OriginalDamage * SplinterDamageScale);
				}

				// 새 핸들로 포장
				FGameplayEffectSpecHandle NewHandle(NewSpec);
				Splinter->InitializeProjectile(NewHandle);
			}
		}
	}
}
