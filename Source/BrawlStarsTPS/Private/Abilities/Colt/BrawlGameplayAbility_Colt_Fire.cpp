// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Colt/BrawlGameplayAbility_Colt_Fire.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlProjectile.h"
#include "BrawlPoolSubsystem.h"
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
	// 1. 코스트 및 쿨다운 확인 및 지불
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. Gameplay Event 대기 (좌/우 총구 발사 이벤트)
	if (UAbilityTask_WaitGameplayEvent* WaitEventTaskL = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireEventTagLeft))
	{
		WaitEventTaskL->EventReceived.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnFireLeftEventReceived);
		WaitEventTaskL->ReadyForActivation();
	}
	
	if (UAbilityTask_WaitGameplayEvent* WaitEventTaskR = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireEventTagRight))
	{
		WaitEventTaskR->EventReceived.AddDynamic(this, &UBrawlGameplayAbility_Colt_Fire::OnFireRightEventReceived);
		WaitEventTaskR->ReadyForActivation();
	}

	// 3. 몽타주 선택 (하이퍼차지 여부에 따라)
	UAnimMontage* MontageToPlay = FireMontage;
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		static FGameplayTag HyperStateTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
		if (ASC->HasMatchingGameplayTag(HyperStateTag))
		{
			if (FireMontage_Hyper) MontageToPlay = FireMontage_Hyper;
		}
	}

	// 4. 몽타주 재생
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
		SpawnProjectile(LeftHandSocket);
		SpawnProjectile(RightHandSocket);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBrawlGameplayAbility_Colt_Fire::OnFireLeftEventReceived(FGameplayEventData Payload)
{
	SpawnProjectile(LeftHandSocket);
}

void UBrawlGameplayAbility_Colt_Fire::OnFireRightEventReceived(FGameplayEventData Payload)
{
	SpawnProjectile(RightHandSocket);
}

void UBrawlGameplayAbility_Colt_Fire::SpawnProjectile(FName AttachParentSocketName)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	TSubclassOf<AActor> ClassToSpawn = GetProjectileClassToSpawn();
	if (!ClassToSpawn) return;

	// 부모 클래스의 유틸리티 함수들을 사용하여 위치와 방향 계산
	FVector MuzzleLocation = GetMuzzleLocation(MuzzleSocketName, AttachParentSocketName);
	FRotator ProjectileRotation = GetAimRotation(MuzzleLocation);
	FTransform SpawnTransform(ProjectileRotation, MuzzleLocation);

	// 데미지 스펙 생성
	FGameplayEffectSpecHandle SpecHandle = MakeDamageSpecHandle(1.0f);

	// 풀 서브시스템을 통한 생성 및 즉시 초기화
	if (UBrawlPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UBrawlPoolSubsystem>())
	{
		PoolSubsystem->GetFromPool(ClassToSpawn, SpawnTransform, Cast<AActor>(Character), Cast<APawn>(Character), 
			[SpecHandle](AActor* InActor)
			{
				if (ABrawlProjectile* Projectile = Cast<ABrawlProjectile>(InActor))
				{
					Projectile->InitializeProjectile(SpecHandle);
				}
			});
		
		// 효과 재생
		PlayGameplayCue(MuzzleLocation, ProjectileRotation.Vector());
	}
	else
	{
		// 폴백: 서브시스템이 없는 경우 (기존 방식)
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, MuzzleLocation, ProjectileRotation, SpawnParams))
		{
			if (ABrawlProjectile* Projectile = Cast<ABrawlProjectile>(SpawnedActor))
			{
				Projectile->InitializeProjectile(SpecHandle);
			}
			PlayGameplayCue(MuzzleLocation, ProjectileRotation.Vector());
		}
	}
}
