// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_UpdateSelfStatus.generated.h"

/**
 * UBTS_UpdateSelfStatus
 * 
 * 캐릭터 자신의 상태(체력 비율 등)를 주기적으로 블랙보드에 업데이트합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTS_UpdateSelfStatus : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTS_UpdateSelfStatus();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// 체력 비율을 저장할 블랙보드 키 (Float)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector HealthRatioKey;
};
