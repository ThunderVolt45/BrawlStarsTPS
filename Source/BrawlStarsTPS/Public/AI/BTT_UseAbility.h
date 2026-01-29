// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTT_UseAbility.generated.h"

/**
 * UBTT_UseAbility
 * 
 * 특정 태그를 가진 어빌리티를 발동시킵니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBTT_UseAbility : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_UseAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// 사용할 어빌리티 태그 (예: Ability.Skill.Gadget)
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTag;

	// 실행 후 성공 반환까지 대기할 것인가? (즉발형은 false, 지속형은 true 권장이나 여기선 Trigger만 함)
	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bWaitForResult = false;
};
