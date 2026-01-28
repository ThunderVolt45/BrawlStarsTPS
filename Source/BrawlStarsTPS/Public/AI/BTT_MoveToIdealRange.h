// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_MoveToIdealRange.generated.h"

/**
 * BTT_MoveToIdealRange
 * 
 * 타겟과의 거리를 AICombatSettings의 PreferredCombatRange에 맞추기 위해 이동하는 태스크.
 * - 타겟이 선호 거리보다 멀면 접근
 * - 타겟이 선호 거리보다 가까우면 후퇴 (Kiting)
 */
UCLASS()
class BRAWLSTARSTPS_API UBTT_MoveToIdealRange : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_MoveToIdealRange();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// 이동 명령 취소 시 정리
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector TargetActorKey;

	// 허용 오차 (선호 거리 +- 이 값 안이면 도착한 것으로 간주)
	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptanceRadius = 50.0f;

	// 좌우 무빙(Strafing) 반경
	UPROPERTY(EditAnywhere, Category = "AI|Randomness")
	float StrafeRadius = 200.0f;

	// 무빙 지점 갱신 주기 (초)
	UPROPERTY(EditAnywhere, Category = "AI|Randomness")
	float StrafeInterval = 1.0f;

	// 후퇴/접근 시 각도 변형 (도)
	UPROPERTY(EditAnywhere, Category = "AI|Randomness")
	float RandomDeviationAngle = 30.0f;

	// 최대 이동 시간 (이 시간이 지나면 성공으로 간주하고 종료)
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxMoveDuration = 3.0f;

private:
	// 다음 스트레이핑 이동 시간
	float NextStrafeTime = 0.0f;
	
	// 태스크 시작 시간 저장용
	float TaskStartTime = 0.0f;
};
