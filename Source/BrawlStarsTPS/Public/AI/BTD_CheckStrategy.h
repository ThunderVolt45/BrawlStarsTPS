// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "AI/BrawlAIStrategy.h" // 변경
#include "BTD_CheckStrategy.generated.h"

/**
 * BTD_CheckStrategy
 * 
 * 블랙보드의 StrategyState(Byte/Enum) 값이 특정 전략과 일치하는지 검사하는 데코레이터.
 * 기본 Blackboard Decorator가 C++ Enum을 잘 인식하지 못할 때 사용.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTD_CheckStrategy : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_CheckStrategy();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	// 검사할 블랙보드 키 (StrategyState)
	UPROPERTY(EditAnywhere, Category = "Condition")
	struct FBlackboardKeySelector StrategyKey;

	// 비교할 전략 값
	UPROPERTY(EditAnywhere, Category = "Condition")
	EBrawlAIStrategy TargetStrategy;
};
