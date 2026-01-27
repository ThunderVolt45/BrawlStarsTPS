// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/BrawlGameplayAbility.h"
#include "BrawlGameplayAbility_Dash.generated.h"

/**
 * UBrawlGameplayAbility_Dash
 * 
 * 캐릭터가 바라보는 방향(또는 입력 방향)으로 빠르게 이동하는 돌진 어빌리티
 * RootMotionSource를 사용하여 정교한 이동 제어 가능
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Dash : public UBrawlGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility_Dash();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 루트 모션 종료 시 호출
	UFUNCTION()
	void OnDashFinished();

protected:
	// 돌진 강도 (속도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	float DashStrength = 1500.0f;

	// 돌진 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	float DashDuration = 0.5f;

	// 돌진 중 사용할 몽타주 (선택 사항)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	TObjectPtr<UAnimMontage> DashMontage;

	// 돌진 중 중력 적용 여부 (공중 돌진 가능 여부 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	bool bEnableGravity = false;

	// 돌진 시작 시 자신에게 적용할 게임플레이 효과 (예: 탄약 충전, 무적 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Effects")
	TSubclassOf<class UGameplayEffect> EffectToApplyOnDash;

	// 효과에 적용할 값 (SetByCaller, 예: 충전량, 회복량)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Effects")
	float EffectMagnitude = 0.0f;

	// 값을 전달할 태그 (예: Data.Amount, Data.Ammo)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Effects")
	FGameplayTag EffectMagnitudeTag;
};
