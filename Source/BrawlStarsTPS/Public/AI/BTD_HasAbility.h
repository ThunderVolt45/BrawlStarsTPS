// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"
#include "BTD_HasAbility.generated.h"

/**
 * UBTD_HasAbility
 * 
 * 특정 태그를 가진 어빌리티를 보유하고 있으며, 
 * 현재 사용 가능한 상태(쿨다운 X, 자원 충분)인지 검사합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTD_HasAbility : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_HasAbility();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	// 확인할 어빌리티 태그 (예: Ability.Skill.Super)
	UPROPERTY(EditAnywhere, Category = "Condition")
	FGameplayTag AbilityTag;
};
