#pragma once

#include "CoreMinimal.h"
#include "Abilities/Colt/BrawlGameplayAbility_Colt_Fire.h"
#include "BrawlGameplayAbility_Colt_Super.generated.h"

/**
 * UBrawlGameplayAbility_Colt_Super
 * 
 * 콜트의 궁극기 (Bullet Storm) 어빌리티
 * - Colt_Fire의 연사 로직을 재사용하되, 탄환 대신 궁극기 게이지를 소모하고 SuperDamage를 적용합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Colt_Super : public UBrawlGameplayAbility_Colt_Fire
{
	GENERATED_BODY()

public:
	UBrawlGameplayAbility_Colt_Super();

	// 궁극기 게이지 확인 및 소모 로직
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// 궁극기 데미지 속성 반환
	virtual FGameplayAttribute GetDamageAttribute() const override;

protected:
	// 궁극기 사용 시 소모할 게이지 양 (기본값: 100.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Super")
	float SuperCostAmount = 100.0f;
};
