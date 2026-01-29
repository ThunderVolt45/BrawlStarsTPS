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

protected:
	// 점프 높이 (Z축 힘)
	UPROPERTY(EditAnywhere, Category = "Jump")
	float JumpZ = 500.0f;

	// 점프 전진 속도 (XY축 힘)
	UPROPERTY(EditAnywhere, Category = "Jump")
	float JumpForwardSpeed = 500.0f;

	// 타겟 반대 방향으로 점프할지 여부
	UPROPERTY(EditAnywhere, Category = "Jump")
	bool bJumpAway = false;

	// 점프 방향 기준이 될 타겟 (설정 안하면 현재 캐릭터 정면 기준)
	UPROPERTY(EditAnywhere, Category = "Jump")
	struct FBlackboardKeySelector TargetKey;

private:
	FVector GetJumpDirection(UBehaviorTreeComponent& OwnerComp, class ACharacter* MyCharacter);
};
