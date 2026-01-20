// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/BrawlGameplayAbility.h"
#include "BrawlGameplayAbility_Hyper.generated.h"

/**
 * UBrawlGameplayAbility_Hyper
 * 
 * 하이퍼차지(Hypercharge) 어빌리티
 * - 별도의 하이퍼차지 게이지 소모
 * - 사용 시 캐릭터 스펙 강화 (버프 GE 적용)
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Hyper : public UBrawlGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility_Hyper();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// 하이퍼차지 발동 시 적용할 버프 효과 (이속, 데미지 증가 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hyper")
	TSubclassOf<UGameplayEffect> HyperBuffEffectClass;

	// 하이퍼차지 활성화 몽타주 (변신 모션 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hyper")
	TObjectPtr<UAnimMontage> HyperActivateMontage;

	// 하이퍼차지 사용 시 필요한 게이지 양
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hyper")
	float HyperCostAmount = 100.0f;
};
