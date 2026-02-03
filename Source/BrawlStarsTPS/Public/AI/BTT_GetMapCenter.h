// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_GetMapCenter.generated.h"

/**
 * BTT_GetMapCenter
 * 
 * 맵의 중심점(또는 현재 안전 구역의 중심)을 구해 블랙보드에 저장하는 태스크
 */
UCLASS()
class BRAWLSTARSTPS_API UBTT_GetMapCenter : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_GetMapCenter();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// 결과를 저장할 블랙보드 키 (Vector)
	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector TargetLocationKey;
};
