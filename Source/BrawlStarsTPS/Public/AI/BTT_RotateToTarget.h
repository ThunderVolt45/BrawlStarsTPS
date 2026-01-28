// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_RotateToTarget.generated.h"

/**
 * BTT_RotateToTarget
 * 
 * Blackboard의 타겟을 향해 회전하는 태스크.
 * - 캐릭터의 회전 설정을 '전투 모드(컨트롤러 회전 따름)'로 강제 전환함.
 * - Pitch Offset을 적용하여 시선을 위/아래로 조절 가능.
 * - 목표 각도에 도달하면 태스크 성공(Succeeded).
 */
UCLASS()
class BRAWLSTARSTPS_API UBTT_RotateToTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_RotateToTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector TargetKey;

	// 목표 각도 도달 허용 오차 (도)
	UPROPERTY(EditAnywhere, Category = "AI")
	float Precision = 5.0f;

	// 회전 속도 (Interp Speed)
	UPROPERTY(EditAnywhere, Category = "AI")
	float RotationSpeed = 5.0f;

	// true: 등속 회전 (기계적), false: 보간 회전 (부드러움, 끝에서 느려짐)
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bUseConstantSpeed = false;

	// 추가할 상하 회전 각도 (양수: 위, 음수: 아래)
	UPROPERTY(EditAnywhere, Category = "AI")
	float PitchOffset = 0.0f;
};
