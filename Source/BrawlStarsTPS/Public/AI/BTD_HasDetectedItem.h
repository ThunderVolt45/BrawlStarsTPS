// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_HasDetectedItem.generated.h"

/**
 * UBTD_HasDetectedItem
 * 
 * AI 컨트롤러가 감지한 아이템(파워 큐브 등)이 있는지 확인하는 데코레이터
 */
UCLASS()
class BRAWLSTARSTPS_API UBTD_HasDetectedItem : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_HasDetectedItem();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
