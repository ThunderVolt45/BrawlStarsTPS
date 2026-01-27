// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Dash.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UBrawlGameplayAbility_Dash::UBrawlGameplayAbility_Dash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 돌진 중에는 다른 이동이나 스킬을 못 쓰도록 태그로 막을 수 있음
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dash")));
}

void UBrawlGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 0. 돌진 시작 시 추가 효과 적용 (탄약 재충전, 무적 등)
	if (EffectToApplyOnDash)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectToApplyOnDash);
		if (SpecHandle.IsValid())
		{
			// SetByCaller 값 주입 (태그가 유효하고 값이 0이 아닐 때)
			if (EffectMagnitudeTag.IsValid() && !FMath::IsNearlyZero(EffectMagnitude))
			{
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(EffectMagnitudeTag, EffectMagnitude);
			}

			(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}

	// 1. 돌진 방향 계산
	// 입력 방향이 있으면 그쪽으로, 없으면 캐릭터 정면으로
	FVector DashDirection = Character->GetLastMovementInputVector();
	if (DashDirection.IsNearlyZero())
	{
		DashDirection = Character->GetActorForwardVector();
	}
	else
	{
		// 입력 방향으로 회전도 시켜줌 (자연스러운 돌진을 위해)
		Character->SetActorRotation(DashDirection.Rotation());
	}

	// 2. 몽타주 재생 (있다면)
	if (DashMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DashMontage);
		// 몽타주 태스크는 단순히 재생용, 종료는 RootMotion이 끝날 때 처리
		if (MontageTask)
		{
			MontageTask->ReadyForActivation();
		}
	}

	// 3. 루트 모션 태스크 생성 (핵심)
	// ApplyRootMotionConstantForce: 지정된 시간 동안 강제로 힘을 가함
	UAbilityTask_ApplyRootMotionConstantForce* RootMotionTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this,
		FName("DashTask"),
		DashDirection,
		DashStrength,
		DashDuration,
		false, // IsAdditive (false면 기존 속도 무시하고 덮어씀 -> 즉발적 돌진)
		nullptr, // Curve (속도 곡선, 필요시 설정)
		ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, // 끝난 후 속도 유지 여부
		FVector::ZeroVector,
		0.0f,
		bEnableGravity // 중력 적용 여부
	);

	if (RootMotionTask)
	{
		RootMotionTask->OnFinish.AddDynamic(this, &UBrawlGameplayAbility_Dash::OnDashFinished);
		RootMotionTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBrawlGameplayAbility_Dash::OnDashFinished()
{
	// 돌진이 끝나면 어빌리티 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
