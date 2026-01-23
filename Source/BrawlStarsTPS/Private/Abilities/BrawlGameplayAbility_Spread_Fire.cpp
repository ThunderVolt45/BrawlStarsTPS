// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Spread_Fire.h"
#include "AbilitySystemComponent.h"
#include "BrawlProjectile.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "BrawlAttributeSet.h"

void UBrawlGameplayAbility_Spread_Fire::SpawnProjectile()
{
	// 0. 유효성 검사
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnProjectile Failed: AvatarActor is not a Character"));
		return;
	}

	// 1. 발사체 클래스 결정 (하이퍼차지 여부 확인)
	TSubclassOf<AActor> ClassToSpawn = ProjectileClass;
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		static FGameplayTag HyperStateTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
		if (ASC->HasMatchingGameplayTag(HyperStateTag) && ProjectileClass_Hyper)
		{
			ClassToSpawn = ProjectileClass_Hyper;
		}
	}

	if (!ClassToSpawn)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnProjectile Failed: ProjectileClass is NULL"));
		return;
	}

	// 2. 발사 시작점 (Muzzle) 설정
	FVector MuzzleLocation = Character->GetActorLocation();
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		if (Mesh->DoesSocketExist(MuzzleSocketName))
		{
			MuzzleLocation = Mesh->GetSocketLocation(MuzzleSocketName);
		}
	}

	// 3. 목표 지점 (Camera Aim) 계산
	FVector BaseTargetLocation = MuzzleLocation + (Character->GetActorForwardVector() * 1000.0f); 

	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		FVector CameraLoc;
		FRotator CameraRot;
		PC->GetPlayerViewPoint(CameraLoc, CameraRot);

		// 카메라 위치에서 레이캐스트 시작
		FVector TraceStart = CameraLoc;
		FVector TraceEnd = CameraLoc + (CameraRot.Vector() * AimMaxRange);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character); // 자신은 무시

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			// 충돌 지점까지의 거리 계산
			float DistanceToHit = (HitResult.ImpactPoint - CameraLoc).Size();
			
			// 최소 사거리보다 가까우면 보정 (거리가 0에 가까울수록 TraceEnd(허공)를 바라봄)
			if (DistanceToHit < AimMinRange)
			{
				float Alpha = FMath::Clamp(DistanceToHit / AimMinRange, 0.0f, 1.0f);
				BaseTargetLocation = FMath::Lerp(TraceEnd, HitResult.ImpactPoint, Alpha);
			}
			else
			{
				BaseTargetLocation = HitResult.ImpactPoint;
			}
		}
		else
		{
			BaseTargetLocation = TraceEnd;
		}
	}

	// 4. 발사 방향 회전 (Muzzle -> Target)
	FRotator BaseRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, BaseTargetLocation);
	FVector BaseDirection = BaseRotation.Vector();

	// 5. 발사체 스폰
	int32 RealCount = FMath::Max(1, ProjectileCount);
	
	// SpreadAngle은 도(Degree) 단위이므로 라디안으로 변환 (VRandCone은 라디안 사용)
	float ConeHalfAngleRad = FMath::DegreesToRadians(SpreadAngle);

	// 시드 랜덤 스트림 (필요 시 멤버 변수로 승격하여 시드 제어 가능)
	FRandomStream WeaponRandomStream(FMath::Rand());

	for (int32 i = 0; i < RealCount; i++)
	{
		// 원뿔 내 무작위 방향 벡터 생성
		FVector RandomDir = WeaponRandomStream.VRandCone(BaseDirection, ConeHalfAngleRad);
		FRotator FinalRotation = RandomDir.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, MuzzleLocation, FinalRotation, SpawnParams);
		
		if (ABrawlProjectile* Projectile = Cast<ABrawlProjectile>(SpawnedActor))
		{
			// GAS 데미지 Spec 생성 및 주입
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
			{
				FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
				ContextHandle.AddSourceObject(this);

				if (DamageEffectClass)
				{
					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);
					if (SpecHandle.IsValid())
					{
						// 데미지 양 설정 (GetDamageAttribute에서 가져옴)
						bool bFound = false;
						float DamageValue = ASC->GetGameplayAttributeValue(GetDamageAttribute(), bFound);
						
						// 못 찾았으면 기본값(DamageAmount) 사용
						if (!bFound) DamageValue = DamageAmount;

						// 데미지 적용
						float FinalDamage = FMath::Abs(DamageValue) * DamagePerPelletScale;

						static FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
						SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, FinalDamage);

						// 발사체에 Spec 주입
						Projectile->InitializeProjectile(SpecHandle);
					}
				}
			}
		}
	}
}
