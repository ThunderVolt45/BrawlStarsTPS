// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IsObstacleDestructible.generated.h"

/**
 * UBTD_IsObstacleDestructible
 * 
 * 타겟과의 사이에 IBrawlDestructibleInterface를 구현한 파괴 가능한 장애물이 있는지 검사합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTD_IsObstacleDestructible : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_IsObstacleDestructible();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	// 검사할 타겟 (블랙보드 키)
	UPROPERTY(EditAnywhere, Category = "Condition")
	struct FBlackboardKeySelector TargetKey;

	// 장애물 판정 채널
	UPROPERTY(EditAnywhere, Category = "Condition")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	
	// 디버그 드로잉
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;
};
