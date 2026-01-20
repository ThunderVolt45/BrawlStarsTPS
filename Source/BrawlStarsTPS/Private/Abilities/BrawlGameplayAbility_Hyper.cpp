// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Hyper.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UBrawlGameplayAbility_Hyper::UBrawlGameplayAbility_Hyper()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UBrawlGameplayAbility_Hyper::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		bool bFound = false;
		float CurrentHyper = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetHyperChargeAttribute(), bFound);
		
		if (bFound && CurrentHyper < HyperCostAmount)
		{
			UE_LOG(LogTemp, Warning, TEXT("HyperCharge Not Ready! Current: %f / %f"), CurrentHyper, HyperCostAmount);
			return false;
		}
		return true;
	}
	return false;
}

void UBrawlGameplayAbility_Hyper::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// 하이퍼차지 게이지 완전 소모 (또는 정해진 양만큼 차감)
		bool bFound = false;
		float CurrentHyper = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetHyperChargeAttribute(), bFound);
		
		ASC->ApplyModToAttributeUnsafe(UBrawlAttributeSet::GetHyperChargeAttribute(), EGameplayModOp::Additive, -CurrentHyper);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, TEXT("HYPERCHARGE ACTIVATED!"));
		}
	}
}

void UBrawlGameplayAbility_Hyper::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 버프 적용
	ApplyEffectToSelf(HyperBuffEffectClass, FGameplayTag(), 1.0f);

	// 2. 몽타주 재생
	if (HyperActivateMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, HyperActivateMontage);
		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UBrawlGameplayAbility::K2_EndAbility);
			MontageTask->OnInterrupted.AddDynamic(this, &UBrawlGameplayAbility::K2_EndAbility);
			MontageTask->OnBlendOut.AddDynamic(this, &UBrawlGameplayAbility::K2_EndAbility);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
