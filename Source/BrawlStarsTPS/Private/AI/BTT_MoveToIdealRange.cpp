// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_MoveToIdealRange.h"
#include "AI/BrawlAIController.h"
#include "BrawlCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"

UBTT_MoveToIdealRange::UBTT_MoveToIdealRange()
{
	NodeName = TEXT("Move To Ideal Range");
	bNotifyTick = true; // Tick 활성화
	
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_MoveToIdealRange, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTT_MoveToIdealRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 첫 실행 시 바로 Tick으로 넘김
	return EBTNodeResult::InProgress;
}

void UBTT_MoveToIdealRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ABrawlCharacter* MyPawn = AIController ? Cast<ABrawlCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AIController || !MyPawn || !Blackboard)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 1. 거리 및 설정 확인
	const FAICombatSettings& Settings = MyPawn->GetAICombatSettings();
	float Distance = MyPawn->GetDistanceTo(TargetActor);
	float PreferredRange = Settings.PreferredCombatRange;

	FVector MyLoc = MyPawn->GetActorLocation();
	FVector TargetLoc = TargetActor->GetActorLocation();

	// 2. 이동 로직 결정
	// A. 너무 가까움 -> 후퇴 (Retreat)
	if (Distance < PreferredRange - AcceptanceRadius)
	{
		// 타겟 반대 방향 벡터
		FVector DirToMe = (MyLoc - TargetLoc).GetSafeNormal();

		if (DirToMe.IsNearlyZero()) DirToMe = MyPawn->GetActorForwardVector() * -1.0f;
		
		// 랜덤 각도 부여 (후퇴 경로 다양화)
		float RandomAngle = FMath::RandRange(-RandomDeviationAngle, RandomDeviationAngle);
		FVector RotatedDir = DirToMe.RotateAngleAxis(RandomAngle, FVector::UpVector);
		
		// 후퇴 목표 지점 계산
		FVector RetreatPos = MyLoc + RotatedDir * 200.0f;
		
		// 네비게이션 메시 위에 투영
		FNavLocation NavLoc;
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

		if (NavSys && NavSys->ProjectPointToNavigation(RetreatPos, NavLoc, FVector(100, 100, 100)))
		{
			AIController->MoveToLocation(NavLoc.Location);
		}
		else
		{
			AIController->StopMovement();
		}
	}
	// B. 너무 멈 -> 접근 (Approach)
	else if (Distance > PreferredRange + AcceptanceRadius)
	{
		// 접근 시에는 확실하게 타겟을 향해 가되, 약간의 오차를 허용하여 자연스럽게 만듦
		// (네비게이션이 알아서 경로를 찾으므로 과도한 랜덤성은 배제)
		FAIMoveRequest MoveReq;
		MoveReq.SetGoalActor(TargetActor);
		MoveReq.SetAcceptanceRadius(PreferredRange);
		
		AIController->MoveTo(MoveReq);
	}
	// C. 적절한 거리 유지 중 -> 좌우 무빙 (Strafing)
	else
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();

		if (CurrentTime >= NextStrafeTime)
		{
			// 타겟 방향 벡터
			FVector DirToTarget = (TargetLoc - MyLoc).GetSafeNormal();
			
			// 타겟을 기준으로 좌/우 벡터 (Cross Product with UpVector)
			FVector RightDir = FVector::CrossProduct(DirToTarget, FVector::UpVector);
			
			// 랜덤하게 좌 또는 우 선택
			bool bGoRight = FMath::RandBool();
			FVector StrafeDir = bGoRight ? RightDir : -RightDir;
			
			// 약간의 전진/후진 섞기 (완벽한 원운동보다는 타원형/불규칙)
			float ForwardBias = FMath::RandRange(-0.5f, 0.5f);
			FVector FinalDir = (StrafeDir + DirToTarget * ForwardBias).GetSafeNormal();
			FVector StrafePos = MyLoc + FinalDir * StrafeRadius;
			
			// 이동 명령
			FNavLocation NavLoc;
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

			if (NavSys && NavSys->ProjectPointToNavigation(StrafePos, NavLoc, FVector(100, 100, 100)))
			{
				AIController->MoveToLocation(NavLoc.Location);
			}
			
			// 다음 무빙 시간 설정
			NextStrafeTime = CurrentTime + FMath::RandRange(StrafeInterval * 0.8f, StrafeInterval * 1.2f);
		}
		
		// 계속 무빙 중이므로 FinishLatentTask를 호출하지 않고 InProgress 유지
		// 공격 Task 등으로 넘어가려면 데코레이터 조건(쿨다운 등)에 의해 중단되거나,
		// 일정 시간 후 성공으로 리턴하는 로직을 추가할 수 있음.
		// 여기서는 "위치 잡기" 자체가 목적이므로 계속 수행.
	}
}

	

void UBTT_MoveToIdealRange::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	// 필요시 정리 작업
}
