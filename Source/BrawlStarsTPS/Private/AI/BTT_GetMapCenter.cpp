// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_GetMapCenter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/GameModeBase.h"
#include "NavigationSystem.h"

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

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	// 1. 중심점 설정 및 유효성 검사
	// TODO: 추후 쇼다운 독구름 축소 로직과 연동하여 '현재 안전 구역의 중심'을 가져오도록 수정 필요
	FVector CenterLocation = FVector::ZeroVector;
	FVector ResultLocation = CenterLocation;

	// 중심점 자체가 네비메쉬 밖에 있을 수 있으므로(벽 속 등), 가장 가까운 유효 지점으로 투영
	FNavLocation CenterNavLoc;
	if (NavSys->ProjectPointToNavigation(CenterLocation, CenterNavLoc, FVector(500.0f, 500.0f, 500.0f)))
	{
		CenterLocation = CenterNavLoc.Location;
	}

	// 2. 도달 가능한(Reachable) 랜덤 위치 탐색
	// GetRandomReachablePointInRadius는 중심점에서 경로가 연결된 지점만 반환합니다.
	FNavLocation RandomPt;
	if (NavSys->GetRandomReachablePointInRadius(CenterLocation, PatrolRadius, RandomPt))
	{
		ResultLocation = RandomPt.Location;
	}
	else
	{
		// 실패 시 즉시 중단
		return EBTNodeResult::Failed;
	}

	// 블랙보드에 저장
	Blackboard->SetValueAsVector(TargetLocationKey.SelectedKeyName, ResultLocation);

	return EBTNodeResult::Succeeded;
}
