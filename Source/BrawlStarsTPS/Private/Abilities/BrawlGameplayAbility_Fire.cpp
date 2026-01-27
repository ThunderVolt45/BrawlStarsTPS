// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Fire.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlCharacter.h"
#include "BrawlProjectile.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UBrawlGameplayAbility_Fire::UBrawlGameplayAbility_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 발사 중임을 나타내는 태그 (Reload 등 다른 동작 차단용)
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire")));
}

void UBrawlGameplayAbility_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("BrawlGameplayAbility_Fire::ActivateAbility Called!"));

	// 1. 코스트 및 쿨다운 확인 및 지불
	// CommitAbility는 Cost GE가 없으면 true를 반환하지만 ApplyCost를 부르지 않음.
	// 따라서 CommitAbility 호출 후, 강제로 ApplyCost를 호출하여 C++ 변수 기반 차감을 수행.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("BrawlGameplayAbility_Fire::ActivateAbility Failed CommitAbility"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 강제 코스트 적용 (GE 설정이 없어도 탄환 깎음)
	// ApplyCost(Handle, ActorInfo, ActivationInfo); // CommitAbility 내부에서 호출되므로 중복 호출 제거

	// 2. Gameplay Event 대기 (Event.Weapon.Fire)
	// 몽타주에서 노티파이로 이벤트를 보내면 OnFireEventReceived가 호출됨
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireEventTag);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnFireEventReceived);
		WaitEventTask->ReadyForActivation();
	}

	// 3. 몽타주 재생
	// PlayMontageAndWait를 쓰면 몽타주 종료 시점까지 어빌리티를 유지할 수 있음
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FireMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnMontageEnded);
		MontageTask->OnBlendOut.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주 재생 실패 시 즉시 발사 시도 (안전장치)
		SpawnProjectile();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBrawlGameplayAbility_Fire::OnFireEventReceived(FGameplayEventData Payload)
{
	// 이벤트 수신 시 발사체 스폰
	SpawnProjectile();
}

void UBrawlGameplayAbility_Fire::SpawnProjectile()
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
		if (ASC->HasMatchingGameplayTag(HyperStateTag))
		{
			if (ProjectileClass_Hyper)
			{
				ClassToSpawn = ProjectileClass_Hyper;
			}
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
	FVector TargetLocation = MuzzleLocation + (Character->GetActorForwardVector() * 1000.0f); // 기본값

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

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, 
			ECC_Visibility, QueryParams))
		{
			// 충돌 지점까지의 거리 계산
			float DistanceToHit = (HitResult.ImpactPoint - CameraLoc).Size();

			// 최소 사거리보다 가까우면 보정 (거리가 0에 가까울수록 TraceEnd(허공)를 바라봄)
			if (DistanceToHit < AimMinRange)
			{
				float Alpha = FMath::Clamp(DistanceToHit / AimMinRange, 0.0f, 1.0f);
				TargetLocation = FMath::Lerp(TraceEnd, HitResult.ImpactPoint, Alpha);
			}
			else
			{
				TargetLocation = HitResult.ImpactPoint;
			}
		}
		else
		{
			TargetLocation = TraceEnd;
		}
	}

	// 4. 발사 방향 계산 (Muzzle -> Target)
	FRotator BaseRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, TargetLocation);
	FVector BaseDirection = BaseRotation.Vector();

	// 5. 발사체 스폰
	int32 RealCount = FMath::Max(1, ProjectileCount);
	
	// SpreadAngle은 도(Degree) 단위이므로 라디안으로 변환 (VRandCone은 라디안 사용)
	float ConeHalfAngleRad = FMath::DegreesToRadians(SpreadAngle);
	
	// 시드 랜덤 스트림
	FRandomStream WeaponRandomStream(FMath::Rand());

	for (int32 i = 0; i < RealCount; i++)
	{
		// 원뿔 내 무작위 방향 벡터 생성
		FVector LaunchDir = ConeHalfAngleRad > UE_SMALL_NUMBER ? 
			WeaponRandomStream.VRandCone(BaseDirection, ConeHalfAngleRad) : BaseDirection;
		
		FRotator FinalRotation = LaunchDir.Rotation();

		// 발사체 소유자 설정
		// 충돌 전 초기화를 위해 Deferred Spawn 사용
		FTransform SpawnTransform(FinalRotation, MuzzleLocation);
		
		AActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(
			ClassToSpawn, 
			SpawnTransform, 
			Character, 
			Character, 
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
		
		ABrawlProjectile* Projectile = Cast<ABrawlProjectile>(SpawnedActor);
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		
		if (Projectile && ASC)
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

					// 데미지 적용 (펠릿 스케일 적용)
					float FinalDamage = FMath::Abs(DamageValue) * DamagePerPelletScale;

					static FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
					SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, FinalDamage);
							
					// 발사체에 Spec 주입 (FinishSpawning 전에 해야 함)
					Projectile->InitializeProjectile(SpecHandle);
				}
			}
		}

		// 최종 스폰 완료 (이때 물리/충돌 시작)
		if (SpawnedActor)
		{
			UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);
		}
	}
}

FGameplayAttribute UBrawlGameplayAbility_Fire::GetDamageAttribute() const
{
	return UBrawlAttributeSet::GetAttackDamageAttribute();
}

void UBrawlGameplayAbility_Fire::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
