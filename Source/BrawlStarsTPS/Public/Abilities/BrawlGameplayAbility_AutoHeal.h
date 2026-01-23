// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/BrawlGameplayAbility.h"
#include "BrawlGameplayAbility_AutoHeal.generated.h"

/**
 * UBrawlGameplayAbility_AutoHeal
 * 
 * 비전투 상태(3초간 공격/피격 없음) 시 체력을 자동으로 회복하는 패시브 어빌리티
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_AutoHeal : public UBrawlGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility_AutoHeal();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 공격 태그 감지 콜백
	void OnCombatTagChanged(const FGameplayTag Tag, int32 NewCount);
	
	// 피격 감지 콜백
	void OnGameplayEffectApplied(UAbilitySystemComponent* Source, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle ActiveHandle);

	// 전투 상태 돌입 (회복 중단, 타이머 리셋)
	void EnterCombatState();

	// 타이머 만료 시 (회복 시작)
	void StartHealing();

	// 회복 틱 (주기적으로 호출)
	void TickHealing();

protected:
	// 비전투 판단 대기 시간 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoHeal")
	float NoCombatDelay = 3.0f;

	// 회복 주기 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoHeal")
	float HealInterval = 1.0f;

	// 초당 회복량 (최대 체력 비례 %)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoHeal")
	float HealPercentPerSec = 0.125f; // 12.5%

	// 감지할 공격 태그들 (이 태그가 붙으면 전투 상태로 간주)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoHeal")
	FGameplayTagContainer CombatTriggerTags;

	// 적용할 회복 GE 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoHeal")
	TSubclassOf<UGameplayEffect> HealEffectClass;

private:
	FTimerHandle TimerHandle_StartHeal;
	FTimerHandle TimerHandle_TickHeal;

	FDelegateHandle OnEffectAppliedDelegateHandle;
	TMap<FGameplayTag, FDelegateHandle> TagDelegateHandles;
};
