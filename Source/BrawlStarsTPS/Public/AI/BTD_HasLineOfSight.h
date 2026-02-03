// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_HasLineOfSight.generated.h"

/**
 * UBTD_HasLineOfSight
 * 
 * 자신과 타겟(Blackboard Key) 사이에 장애물이 없는지 검사하는 데코레이터입니다.
 * Line Trace를 사용하여 시야가 확보되었는지 확인합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTD_HasLineOfSight : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_HasLineOfSight();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	// 검사할 타겟 (블랙보드 키)
	UPROPERTY(EditAnywhere, Category = "Condition")
	struct FBlackboardKeySelector TargetKey;

	// 트레이스 채널 (기본: Visibility)
	UPROPERTY(EditAnywhere, Category = "Condition")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	
	// 디버그 드로잉 여부
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;
};
