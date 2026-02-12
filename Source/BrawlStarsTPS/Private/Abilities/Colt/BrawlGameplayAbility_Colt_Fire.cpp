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
	// UE_LOG(LogTemp, Warning, TEXT("BrawlGameplayAbility_Colt_Fire::ActivateAbility Called!"));

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
	if (UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FireEventTagLeft))
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnFireLeftEventReceived);
		WaitEventTask->ReadyForActivation();
	}
	
	if (UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FireEventTagRight))
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

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay);
	
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
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	// 1. 발사체 클래스
	TSubclassOf<AActor> ClassToSpawn = GetProjectileClassToSpawn();
	if (!ClassToSpawn)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnProjectile Failed: ProjectileClass is NULL"));
		return;
	}

	// 2. 발사 시작점
	FVector MuzzleLocation = GetMuzzleLocation(MuzzleSocketName, AttachParentSocketName);

	// 3. 발사 방향
	FRotator ProjectileRotation = GetAimRotation(MuzzleLocation);

	// 4. 발사체 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;

	AActor* SpawnedActor = GetWorld()->SpawnActor<
		AActor>(ClassToSpawn, MuzzleLocation, ProjectileRotation, SpawnParams);
	if (ABrawlProjectile* Projectile = Cast<ABrawlProjectile>(SpawnedActor))
	{
		// GAS 데미지 Spec 생성 및 주입
		FGameplayEffectSpecHandle SpecHandle = MakeDamageSpecHandle(1.0f); // 1.0 Scale
		if (SpecHandle.IsValid())
		{
			Projectile->InitializeProjectile(SpecHandle);
		}

		// SFX/VFX 재생 (블루프린트에서 설정한 AbilityGameplayCueTag 사용)
		PlayGameplayCue(MuzzleLocation, ProjectileRotation.Vector());
	}
}
		