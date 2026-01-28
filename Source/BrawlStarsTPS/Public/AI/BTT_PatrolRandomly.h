// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_PatrolRandomly.generated.h"

/**
 * BTT_PatrolRandomly
 * 
 * 현재 위치를 기준으로 일정 반경 내의 랜덤한 지점으로 이동하는 태스크.
 * 이동이 완료되면 성공(Succeeded)을 반환함.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTT_PatrolRandomly : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_PatrolRandomly();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
	// 순찰 반경
	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolRadius = 500.0f;

	// 이동 허용 오차
	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptanceRadius = 100.0f;
};
