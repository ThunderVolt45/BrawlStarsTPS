// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_EvaluateStrategy.generated.h"

/**
 * BTS_EvaluateStrategy
 * 
 * 주기적으로 실행되어 AI의 현재 상황(체력, 타겟과의 거리)을 분석하고
 * 적절한 전략(Strategy)을 블랙보드에 업데이트하는 서비스
 */
UCLASS()
class BRAWLSTARSTPS_API UBTS_EvaluateStrategy : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTS_EvaluateStrategy();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// 블랙보드 키 선택 (에디터에서 설정)
	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector StrategyStateKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector DistanceToTargetKey;
};
