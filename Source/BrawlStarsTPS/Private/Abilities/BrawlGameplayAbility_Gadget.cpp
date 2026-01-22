// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Gadget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"

UBrawlGameplayAbility_Gadget::UBrawlGameplayAbility_Gadget()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UBrawlGameplayAbility_Gadget::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// 가젯은 탄환(Ammo)을 체크하지 않음
	// 만약 가젯만의 특별한 자원이 있다면 여기서 체크
	return true;
}

void UBrawlGameplayAbility_Gadget::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 가젯은 탄환을 소모하지 않음
}

FGameplayAttribute UBrawlGameplayAbility_Gadget::GetDamageAttribute() const
{
	return UBrawlAttributeSet::GetGadgetDamageAttribute();
}