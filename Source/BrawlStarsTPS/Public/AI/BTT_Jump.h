// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Jump.generated.h"

/**
 * UBTT_Jump
 * 
 * AI 캐릭터에게 점프를 명령하는 태스크 노드입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTT_Jump : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_Jump();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
