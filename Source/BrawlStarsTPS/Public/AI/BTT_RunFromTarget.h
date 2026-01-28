// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_RunFromTarget.generated.h"

/**
 * BTT_RunFromTarget
 * 
 * 타겟으로부터 멀어지는 방향으로 일정 시간 동안 도주하는 태스크.
 * 체력이 낮거나 위험한 상황에서 사용하기 적합함.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTT_RunFromTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_RunFromTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector TargetActorKey;

	// 한 번의 이동 명령으로 가려고 시도하는 거리
	UPROPERTY(EditAnywhere, Category = "AI")
	float FleeDistance = 600.0f;

	// 총 도주 지속 시간
	UPROPERTY(EditAnywhere, Category = "AI")
	float RunDuration = 4.0f;

	// 경로 재탐색 주기 (초)
	UPROPERTY(EditAnywhere, Category = "AI")
	float PathUpdateInterval = 1.0f;

	// 도주 방향의 무작위 변동폭 (도 단위, +/- 이 값만큼 랜덤 회전)
	UPROPERTY(EditAnywhere, Category = "AI")
	float RandomAngle = 15.0f;

private:
	float TaskStartTime = 0.0f;
	float LastPathUpdateTime = 0.0f;
};
