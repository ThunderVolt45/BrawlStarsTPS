// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/BrawlGameplayAbility.h"
#include "BrawlGameplayAbility_Reload.generated.h"

/**
 * UBrawlGameplayAbility_Reload
 * 
 * 브롤스타즈 스타일의 자동 재장전 어빌리티입니다.
 * - 패시브로 동작하거나 수동으로 활성화 가능
 * - 탄환이 MaxAmmo보다 적으면 일정 간격으로 탄환을 회복
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Reload : public UBrawlGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility_Reload();

	// 어빌리티 활성화 로직
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 어빌리티 종료 로직
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 한 발 재장전 시도
	void TryReloadToken();

	// 재장전 완료 후 처리 (탄환 증가)
	void CommitReload();

	// 탄환이 변경되었을 때 호출 (재장전 필요 여부 재확인)
	void OnAmmoAttributeChanged(const FOnAttributeChangeData& Data);

protected:
	// 재장전 1회당 걸리는 시간 (초) - 어트리뷰트에서 가져올 수도 있지만 기본값 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	float ReloadDuration = 1.0f;

	// 한 번에 충전할 탄환 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	float ReloadAmount = 1.0f;

	// 타이머 핸들
	FTimerHandle ReloadTimerHandle;
};
