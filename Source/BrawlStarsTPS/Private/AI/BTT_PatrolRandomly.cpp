// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_PatrolRandomly.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTT_PatrolRandomly::UBTT_PatrolRandomly()
{
	NodeName = TEXT("Patrol Randomly");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_PatrolRandomly::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* MyPawn = AIController ? AIController->GetPawn() : nullptr;

	if (!AIController || !MyPawn)
	{
		return EBTNodeResult::Failed;
	}

	// 순찰 시 자연스러운 회전을 위한 설정
	// 1. 기존의 포커스(시선 고정) 해제
	AIController->ClearFocus(EAIFocusPriority::Gameplay);

	// 2. 캐릭터가 이동 방향을 바라보도록 설정
	ACharacter* MyCharacter = Cast<ACharacter>(MyPawn);
	if (MyCharacter)
	{
		MyCharacter->bUseControllerRotationYaw = false;
		if (MyCharacter->GetCharacterMovement())
		{
			MyCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
		}
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	// 현재 위치 기준 랜덤 포인트 찾기
	FNavLocation RandomLocation;
	bool bFound = NavSys->GetRandomReachablePointInRadius(MyPawn->GetActorLocation(), PatrolRadius, RandomLocation);

	if (bFound)
	{
		// 이동 명령 수행
		FAIMoveRequest MoveReq;
		MoveReq.SetGoalLocation(RandomLocation.Location);
		MoveReq.SetAcceptanceRadius(AcceptanceRadius);
		
		FPathFollowingRequestResult Result = AIController->MoveTo(MoveReq);
		
		if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
		{
			return EBTNodeResult::InProgress;
		}
	}

	return EBTNodeResult::Failed;
}

void UBTT_PatrolRandomly::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 이동 상태 확인
	EPathFollowingStatus::Type Status = AIController->GetMoveStatus();

	if (Status == EPathFollowingStatus::Idle)
	{
		// 이동이 끝남 (성공적으로 도착했거나, 다른 이유로 멈춤)
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTT_PatrolRandomly::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
		AIController->GetPawn()->SetActorRotation(AIController->GetControlRotation());
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTT_PatrolRandomly::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
