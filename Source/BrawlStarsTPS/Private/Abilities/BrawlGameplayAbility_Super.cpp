// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Super.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"

UBrawlGameplayAbility_Super::UBrawlGameplayAbility_Super()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UBrawlGameplayAbility_Super::CheckCost(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		bool bFoundCharge = false;
		bool bFoundCost = false;
		
		float CurrentSuperCharge = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetSuperChargeAttribute(), bFoundCharge);
		float RequiredCost = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetSuperCostAttribute(), bFoundCost);
		
		if (bFoundCharge && bFoundCost && CurrentSuperCharge < RequiredCost)
		{
			UE_LOG(LogTemp, Warning, TEXT("Super Not Charged! Current: %f / Required: %f"), CurrentSuperCharge, RequiredCost);
			return false;
		}
	}

	return true;
}

void UBrawlGameplayAbility_Super::ApplyCost(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 궁극기 게이지 소모
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// 게이지를 0으로 덮어씌운다
		ASC->ApplyModToAttributeUnsafe(UBrawlAttributeSet::GetSuperChargeAttribute(), EGameplayModOp::Override, 0);
	}
}

void UBrawlGameplayAbility_Super::ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Super는 BrawlGameplayAbility_Fire를 상속받았으므로
	// 부모의 ActivateAbility(CommitAbility 호출 및 발사체 로직)가 그대로 동작함
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
