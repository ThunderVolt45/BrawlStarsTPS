// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_MoveToIdealRange.h"
#include "AI/BrawlAIController.h"
#include "BrawlCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "Environment/BrawlPoisonZone.h"
#include "Kismet/GameplayStatics.h"

UBTT_MoveToIdealRange::UBTT_MoveToIdealRange()
{
	NodeName = TEXT("Move To Ideal Range");
	bNotifyTick = true; // Tick 활성화
	
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_MoveToIdealRange, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTT_MoveToIdealRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 첫 실행 시 바로 Tick으로 넘김
	TaskStartTime = GetWorld()->GetTimeSeconds();
	return EBTNodeResult::InProgress;
}

void UBTT_MoveToIdealRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 시간 초과 체크
	if (GetWorld()->GetTimeSeconds() - TaskStartTime >= MaxMoveDuration)
	{
		// 시간이 지나면 성공으로 간주하고 종료 (트리가 다시 평가되도록)
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

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

	// 타겟이 상자(Etc)라면 이상 거리를 100으로 잡아서 바짝 붙도록 함
	if (ABrawlCharacter* TargetBrawler = Cast<ABrawlCharacter>(TargetActor))
	{
		if (TargetBrawler->GetCharacterType() == EBrawlCharacterType::Etc)
		{
			PreferredRange = 100.0f;
		}
	}

	FVector MyLoc = MyPawn->GetActorLocation();
	FVector TargetLoc = TargetActor->GetActorLocation();

	// 독구름(안전 구역) 정보 가져오기
	ABrawlPoisonZone* PoisonZone = Cast<ABrawlPoisonZone>(UGameplayStatics::GetActorOfClass(GetWorld(), ABrawlPoisonZone::StaticClass()));

	// 0. 시야 확인 (옵션)
	bool bHasLoS = true;
	if (bCheckLineOfSight)
	{
		FVector StartLocation = MyPawn->GetPawnViewLocation();
		
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(MyPawn);

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			TargetLoc,
			TraceChannel,
			Params
		);

		if (bHit && HitResult.GetActor() != TargetActor)
		{
			bHasLoS = false;
		}
	}

	// 2. 이동 로직 결정
	
	// A. 시야가 가려짐 -> 무조건 접근 (Move to see)
	if (!bHasLoS)
	{
		FVector MovePos = TargetLoc;
		if (PoisonZone)
		{
			MovePos = PoisonZone->GetClosestSafePosition(TargetLoc);
		}

		FAIMoveRequest MoveReq;
		MoveReq.SetGoalLocation(MovePos);
		MoveReq.SetAcceptanceRadius(50.0f);
		AIController->MoveTo(MoveReq);
	}
	// B. 너무 가까움 -> 후퇴 (Retreat)
	else if (Distance < PreferredRange - AcceptanceRadius)
	{
		FVector DirToMe = (MyLoc - TargetLoc).GetSafeNormal();
		if (DirToMe.IsNearlyZero()) DirToMe = MyPawn->GetActorForwardVector() * -1.0f;
		
		float RandomAngle = FMath::RandRange(-RandomDeviationAngle, RandomDeviationAngle);
		FVector RotatedDir = DirToMe.RotateAngleAxis(RandomAngle, FVector::UpVector);
		FVector RetreatPos = MyLoc + RotatedDir * 200.0f;
		
		// 안전 구역 강제 고정
		if (PoisonZone)
		{
			RetreatPos = PoisonZone->GetClosestSafePosition(RetreatPos);
		}

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
		FVector MovePos = TargetLoc;
		if (PoisonZone)
		{
			MovePos = PoisonZone->GetClosestSafePosition(TargetLoc);
		}

		FAIMoveRequest MoveReq;
		MoveReq.SetGoalLocation(MovePos);
		MoveReq.SetAcceptanceRadius(PreferredRange);
		AIController->MoveTo(MoveReq);
	}
	// C. 적절한 거리 유지 중 -> 좌우 무빙 (Strafing)
	else
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime >= NextStrafeTime)
		{
			FVector DirToTarget = (TargetLoc - MyLoc).GetSafeNormal();
			FVector RightDir = FVector::CrossProduct(DirToTarget, FVector::UpVector);
			
			bool bGoRight = FMath::RandBool();
			FVector StrafeDir = bGoRight ? RightDir : -RightDir;
			
			float ForwardBias = FMath::RandRange(-0.5f, 0.5f);
			FVector FinalDir = (StrafeDir + DirToTarget * ForwardBias).GetSafeNormal();
			FVector StrafePos = MyLoc + FinalDir * StrafeRadius;
			
			// 안전 구역 강제 고정
			if (PoisonZone)
			{
				StrafePos = PoisonZone->GetClosestSafePosition(StrafePos);
			}

			FNavLocation NavLoc;
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
			if (NavSys && NavSys->ProjectPointToNavigation(StrafePos, NavLoc, FVector(100, 100, 100)))
			{
				AIController->MoveToLocation(NavLoc.Location);
			}
			
			NextStrafeTime = CurrentTime + FMath::RandRange(StrafeInterval * 0.8f, StrafeInterval * 1.2f);
		}
	}
}

	

void UBTT_MoveToIdealRange::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	// 필요시 정리 작업
}
