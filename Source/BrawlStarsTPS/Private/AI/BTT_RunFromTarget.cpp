// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_RunFromTarget.h"
#include "AI/BrawlAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "AIController.h"

UBTT_RunFromTarget::UBTT_RunFromTarget()
{
	NodeName = TEXT("Run From Target");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_RunFromTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TaskStartTime = GetWorld()->GetTimeSeconds();
	// 시작 시 즉시 업데이트가 발생하도록 초기화
	LastPathUpdateTime = -PathUpdateInterval;
	
	return EBTNodeResult::InProgress;
}

void UBTT_RunFromTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 1. 전체 지속 시간 체크
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - TaskStartTime >= RunDuration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 2. 주기적인 경로 재탐색
	if (CurrentTime - LastPathUpdateTime >= PathUpdateInterval)
	{
		LastPathUpdateTime = CurrentTime;

		AAIController* AIController = OwnerComp.GetAIOwner();
		APawn* MyPawn = AIController ? AIController->GetPawn() : nullptr;
		UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

		if (!AIController || !MyPawn || !Blackboard)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (!TargetActor)
		{
			// 타겟이 없으면 도주할 필요가 없으므로 성공 처리 (또는 실패 처리, 상황에 따라 다름)
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		FVector MyLoc = MyPawn->GetActorLocation();
		FVector TargetLoc = TargetActor->GetActorLocation();

		// 타겟 반대 방향 벡터 계산
		FVector DirToTarget = (TargetLoc - MyLoc);
		FVector FleeDir = -DirToTarget.GetSafeNormal();

		// 만약 타겟과 겹쳐있거나 거리가 0이면 임의 방향 설정
		if (FleeDir.IsNearlyZero())
		{
			FleeDir = MyPawn->GetActorForwardVector() * -1.0f;
		}
		else
		{
			// 도주 방향에 약간의 랜덤성 추가 (Z축 기준 회전)
			float ActualRandomAngle = FMath::RandRange(-RandomAngle, RandomAngle);
			FleeDir = FleeDir.RotateAngleAxis(ActualRandomAngle, FVector::UpVector);
		}

		// 목표 지점 계산
		FVector FleePos = MyLoc + FleeDir * FleeDistance;

		// 네비게이션 시스템을 통해 유효한 위치인지 확인 및 보정
		FNavLocation NavLoc;
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

		if (NavSys)
		{
			// 목표 지점 반경 200 내의 네비게이션 가능한 위치 찾기
			if (NavSys->ProjectPointToNavigation(FleePos, NavLoc, FVector(200, 200, 200)))
			{
				AIController->MoveToLocation(NavLoc.Location);
			}
			else
			{
				// 정확한 반대 방향에 길이 없다면?
				// 조금 덜 엄격하게, 반대 방향 기준 부채꼴 범위 등에서 랜덤 포인트를 찾을 수도 있지만
				// 여기서는 단순히 현재 위치 근처에서 랜덤 도주를 시도하거나 멈추는 대신,
				// 네비게이션 투영 범위를 좀 더 넓혀서 재시도
				if (NavSys->ProjectPointToNavigation(FleePos, NavLoc, FVector(500, 500, 500)))
				{
					AIController->MoveToLocation(NavLoc.Location);
				}
			}
		}
	}
}
