// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTT_BrawlAttack.generated.h"

/**
 * BTT_BrawlAttack
 * 
 * GAS 어빌리티를 트리거하여 공격을 수행하는 태스크
 */
UCLASS()
class BRAWLSTARSTPS_API UBTT_BrawlAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_BrawlAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector TargetActorKey;

	// 실행할 어빌리티 태그 (예: Ability.Attack.Primary)
	UPROPERTY(EditAnywhere, Category = "AI")
	FGameplayTag AbilityTag;

	// 공격 후 대기 시간 (딜레이)
	UPROPERTY(EditAnywhere, Category = "AI")
	float PostAttackDelay = 0.5f;

	// 조준 시 상하 회전(Pitch) 오프셋 (양수면 위쪽)
	UPROPERTY(EditAnywhere, Category = "AI")
	float AimPitchOffset = 10.0f;
};
