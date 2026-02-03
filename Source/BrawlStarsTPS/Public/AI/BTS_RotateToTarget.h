// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_RotateToTarget.generated.h"

/**
 * BTS_RotateToTarget
 * 
 * 활성화된 동안 계속해서 타겟(Actor or Vector)을 바라보도록 컨트롤러 회전을 조정하는 서비스입니다.
 * Task가 아니라 Service이므로 실행 흐름을 차단하지 않습니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTS_RotateToTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTS_RotateToTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// 바라볼 타겟 키 (Actor 또는 Vector)
	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector TargetKey;

	// 회전 속도 (Interpolation Speed)
	UPROPERTY(EditAnywhere, Category = "AI")
	float RotationSpeed = 2.0f;
	
	// 등속 회전 여부 (False면 부드러운 감속 회전)
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bUseConstantSpeed = false;

	// 목표 각도와의 오차 허용 범위 (도) - 이 값보다 작으면 회전 멈춤 (떨림 방지)
	UPROPERTY(EditAnywhere, Category = "AI")
	float Precision = 2.0f;

	// 디버그 드로잉 여부
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;
};
