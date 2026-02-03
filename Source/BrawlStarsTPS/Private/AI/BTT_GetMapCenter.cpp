// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_GetMapCenter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/GameModeBase.h"

UBTT_GetMapCenter::UBTT_GetMapCenter()
{
	NodeName = TEXT("Get Map Center");
	
	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_GetMapCenter, TargetLocationKey));
}

EBTNodeResult::Type UBTT_GetMapCenter::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	// 기본값: (0, 0, 0)
	// TODO: 추후 쇼다운 독구름 축소 로직과 연동하여 '현재 안전 구역의 중심'을 가져오도록 수정 필요
	FVector CenterLocation = FVector::ZeroVector;

	// 블랙보드에 저장
	Blackboard->SetValueAsVector(TargetLocationKey.SelectedKeyName, CenterLocation);

	return EBTNodeResult::Succeeded;
}
