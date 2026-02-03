// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTD_HasDetectedItem.h"
#include "AI/BrawlAIController.h"

UBTD_HasDetectedItem::UBTD_HasDetectedItem()
{
	NodeName = TEXT("Has Detected Item");
}

bool UBTD_HasDetectedItem::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	ABrawlAIController* AIController = Cast<ABrawlAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return false;
	}

	return AIController->HasDetectedItems();
}
