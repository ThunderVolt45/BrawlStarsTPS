// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/BrawlGameplayAbility_Fire.h"
#include "BrawlGameplayAbility_Super.generated.h"

/**
 * UBrawlGameplayAbility_Super
 * 
 * 궁극기(Super) 어빌리티의 베이스 클래스
 * - SuperCharge 게이지가 가득 찼는지 확인
 * - 사용 시 게이지 소모
 * - 기본적으로 발사체/공격 형태이므로 BrawlGameplayAbility_Fire를 상속
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Super : public UBrawlGameplayAbility_Fire
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility_Super();

	// 궁극기 게이지 확인 로직 추가
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 궁극기 사용 시 게이지 소모
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 필요한 게이지 양 (보통 100.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Super")
	float SuperCostAmount = 100.0f;
};
