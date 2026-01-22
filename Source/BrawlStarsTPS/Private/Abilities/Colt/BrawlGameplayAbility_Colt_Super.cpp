#include "Abilities/Colt/BrawlGameplayAbility_Colt_Super.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"

UBrawlGameplayAbility_Colt_Super::UBrawlGameplayAbility_Colt_Super()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UBrawlGameplayAbility_Colt_Super::CheckCost(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 부모(Colt_Fire)의 CheckCost는 탄환을 검사하므로 호출하지 않음

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		bool bFound = false;
		float CurrentSuper = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetSuperChargeAttribute(), bFound);
		
		// 궁극기 게이지가 부족하면 발동 불가
		if (bFound && CurrentSuper < SuperCostAmount)
		{
			return false;
		}
	}
	
	return true;
}

void UBrawlGameplayAbility_Colt_Super::ApplyCost(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 부모(Colt_Fire)의 ApplyCost는 탄환을 소모하므로 호출하지 않음

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// 궁극기 게이지 소모
		ASC->ApplyModToAttributeUnsafe(UBrawlAttributeSet::GetSuperChargeAttribute(), EGameplayModOp::Override, 0);
	}
}

FGameplayAttribute UBrawlGameplayAbility_Colt_Super::GetDamageAttribute() const
{
	return UBrawlAttributeSet::GetSuperDamageAttribute();
}
