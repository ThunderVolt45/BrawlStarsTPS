// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Colt/BrawlGameplayAbility_Colt_Fire.h"
#include "BrawlGameplayAbility_Colt_Gadget.generated.h"

/**
 * 
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Colt_Gadget : public UBrawlGameplayAbility_Colt_Fire
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility_Colt_Gadget();

	// 가젯은 탄환을 소모하지 않음
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
protected:
	// 가젯 효과 발동 (Blueprint에서 오버라이드하여 시각적 효과 등 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|Gadget")
	void OnGadgetActivated();
};
