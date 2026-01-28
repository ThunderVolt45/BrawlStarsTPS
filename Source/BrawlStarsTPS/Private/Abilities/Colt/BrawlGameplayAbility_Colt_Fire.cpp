// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Colt/BrawlGameplayAbility_Colt_Fire.h"

#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlProjectile.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UBrawlGameplayAbility_Colt_Fire::UBrawlGameplayAbility_Colt_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UBrawlGameplayAbility_Colt_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("BrawlGameplayAbility_Colt_Fire::ActivateAbility Called!"));

	FGameplayTagContainer RelevantTags;

	// 1. 쿨다운 체크
	if (!CheckCooldown(Handle, ActorInfo, &RelevantTags))
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateAbility Failed: Cooldown"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. 코스트 체크
	if (!CheckCost(Handle, ActorInfo, &RelevantTags))
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateAbility Failed: Cost"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 3. 쿨다운 및 코스트 적용 (수동 호출로 중복 방지)
	ApplyCooldown(Handle, ActorInfo, ActivationInfo);
	ApplyCost(Handle, ActorInfo, ActivationInfo);

	// 2. Gameplay Event 대기 (Event.Weapon.Fire)
	// 몽타주에서 노티파이로 이벤트를 보내면 OnFireEventReceived가 호출됨
	if (UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireEventTagLeft))
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnFireLeftEventReceived);
		WaitEventTask->ReadyForActivation();
	}
	
	if (UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireEventTagRight))
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnFireRightEventReceived);
		WaitEventTask->ReadyForActivation();
	}

	// 3. 몽타주 재생
	// PlayMontageAndWait를 쓰면 몽타주 종료 시점까지 어빌리티를 유지할 수 있음
	UAnimMontage* MontageToPlay = FireMontage;
	
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		static FGameplayTag HyperStateTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
		if (ASC->HasMatchingGameplayTag(HyperStateTag))
		{
			if (FireMontage_Hyper)
			{
				MontageToPlay = FireMontage_Hyper;
			}
		}
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnMontageEnded);
		MontageTask->OnBlendOut.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주 재생 실패 시 즉시 발사 시도 (안전장치)
		SpawnProjectile(LeftHandSocket);
		SpawnProjectile(RightHandSocket);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBrawlGameplayAbility_Colt_Fire::OnFireLeftEventReceived(FGameplayEventData Payload)
{
	// 이벤트 수신 시 발사체 스폰 (왼손 무기)
	SpawnProjectile(LeftHandSocket);
}

void UBrawlGameplayAbility_Colt_Fire::OnFireRightEventReceived(FGameplayEventData Payload)
{
	// 이벤트 수신 시 발사체 스폰 (오른손 무기)
	SpawnProjectile(RightHandSocket);
}

void UBrawlGameplayAbility_Colt_Fire::SpawnProjectile(FName AttachParentSocketName)
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

	// 2. 발사 시작점 (Muzzle) 찾기
	FVector MuzzleLocation = Character->GetActorLocation();
	bool bSocketFound = false;

	// 2-1. 메인 메쉬 확인 (AttachParentSocketName이 없거나 일치할 때만)
	if (AttachParentSocketName.IsNone()) 
	{
		if (USkeletalMeshComponent* MainMesh = Character->GetMesh())
		{
			if (MainMesh->DoesSocketExist(MuzzleSocketName))
			{
				MuzzleLocation = MainMesh->GetSocketLocation(MuzzleSocketName);
				bSocketFound = true;
			}
		}
	}

	// 2-2. 직접 붙은 컴포넌트(Mesh Component) 확인
	// (블루프린트 컴포넌트 패널에서 메쉬 아래에 자식으로 붙이고 Parent Socket을 설정한 경우)
	if (!bSocketFound)
	{
		TArray<UMeshComponent*> MeshComponents;
		Character->GetComponents<UMeshComponent>(MeshComponents);

		for (UMeshComponent* MeshComp : MeshComponents)
		{
			// 부모 소켓 확인 (hand_l에 붙어있는 컴포넌트인가?)
			if (!AttachParentSocketName.IsNone())
			{
				if (MeshComp->GetAttachSocketName() != AttachParentSocketName)
				{
					continue;
				}
			}

			// 소켓 존재 확인
			if (MeshComp->DoesSocketExist(MuzzleSocketName))
			{
				MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
				bSocketFound = true;
				break;
			}
		}
	}

	if (!bSocketFound)
	{
		// 못 찾았으면 기본 위치 (하지만 경고는 띄움)
		UE_LOG(LogTemp, Warning, TEXT("SpawnProjectile: Socket [%s] (Parent: %s) not found! Using ActorLocation."), 
			*MuzzleSocketName.ToString(), *AttachParentSocketName.ToString());
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

	// 4. 발사 방향 회전 (Muzzle -> Target)
	FRotator ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, TargetLocation);

	// 5. 발사체 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, MuzzleLocation, ProjectileRotation, SpawnParams);
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
					float FinalDamage = FMath::Abs(DamageValue);

					static FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
					SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, FinalDamage);
					
					// 발사체에 Spec 주입
					Projectile->InitializeProjectile(SpecHandle);
				}
			}
		}
	}
}

FGameplayAttribute UBrawlGameplayAbility_Colt_Fire::GetDamageAttribute() const
{
	return UBrawlAttributeSet::GetAttackDamageAttribute();
}

void UBrawlGameplayAbility_Colt_Fire::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
