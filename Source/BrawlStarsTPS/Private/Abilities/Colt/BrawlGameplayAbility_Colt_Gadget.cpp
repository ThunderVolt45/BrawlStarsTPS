#include "Abilities/Colt/BrawlGameplayAbility_Colt_Gadget.h"
#include "BrawlAttributeSet.h"

UBrawlGameplayAbility_Colt_Gadget::UBrawlGameplayAbility_Colt_Gadget()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UBrawlGameplayAbility_Colt_Gadget::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 가젯은 탄환(Ammo)을 체크하지 않음
	return true;
}

void UBrawlGameplayAbility_Colt_Gadget::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 가젯은 탄환을 소모하지 않음
}

FGameplayAttribute UBrawlGameplayAbility_Colt_Gadget::GetDamageAttribute() const
{
	static FGameplayTag Gadget2Tag = FGameplayTag::RequestGameplayTag(FName("InputTag.Ability.Gadget2"));
	
	if (StartupInputTag.MatchesTag(Gadget2Tag))
	{
		return UBrawlAttributeSet::GetGadget2DamageAttribute();
	}
	
	return UBrawlAttributeSet::GetGadgetDamageAttribute();
}