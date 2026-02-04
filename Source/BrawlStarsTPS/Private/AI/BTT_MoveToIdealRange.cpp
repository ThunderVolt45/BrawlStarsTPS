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
		// 만약 타겟이 독구름 안에 있다면, 무리하게 접근하지 않음
		bool bIsTargetSafe = PoisonZone ? PoisonZone->IsPositionSafe(TargetLoc) : true;

		if (bIsTargetSafe)
		{
			FAIMoveRequest MoveReq;
			MoveReq.SetGoalActor(TargetActor);
			MoveReq.SetAcceptanceRadius(50.0f); // 최대한 가까이 가서 시야 확보 시도
			AIController->MoveTo(MoveReq);
		}
		else
		{
			// 타겟이 독에 있다면, 안전 구역 내에서 타겟과 가장 가까운 지점으로 이동 시도
			FVector DirToTarget = (TargetLoc - MyLoc).GetSafeNormal();
			FVector EdgePos = MyLoc + DirToTarget * 100.0f; // 조금씩 전진

			if (PoisonZone && !PoisonZone->IsPositionSafe(EdgePos))
			{
				AIController->StopMovement();
			}
			else
			{
				AIController->MoveToLocation(EdgePos);
			}
		}
	}
	// B. 너무 가까움 -> 후퇴 (Retreat)
	else if (Distance < PreferredRange - AcceptanceRadius)
	{
		// 타겟 반대 방향 벡터
		FVector DirToMe = (MyLoc - TargetLoc).GetSafeNormal();

		if (DirToMe.IsNearlyZero()) DirToMe = MyPawn->GetActorForwardVector() * -1.0f;
		
		// 랜덤 각도 부여 (후퇴 경로 다양화)
		float RandomAngle = FMath::RandRange(-RandomDeviationAngle, RandomDeviationAngle);
		FVector RotatedDir = DirToMe.RotateAngleAxis(RandomAngle, FVector::UpVector);
		
		// 후퇴 목표 지점 계산
		FVector RetreatPos = MyLoc + RotatedDir * 200.0f;
		
		// 독구름 체크: 후퇴 지점이 위험하다면 안전 구역 쪽으로 보정
		if (PoisonZone && !PoisonZone->IsPositionSafe(RetreatPos))
		{
			FVector DirToSafe = (PoisonZone->GetActorLocation() - MyLoc).GetSafeNormal();
			RetreatPos = MyLoc + (RotatedDir * 0.5f + DirToSafe * 0.5f).GetSafeNormal() * 200.0f;
		}

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
		// 접근 시 타겟이 독구름 안인지 체크
		bool bIsTargetSafe = PoisonZone ? PoisonZone->IsPositionSafe(TargetLoc) : true;
		
		if (bIsTargetSafe)
		{
			FAIMoveRequest MoveReq;
			MoveReq.SetGoalActor(TargetActor);
			MoveReq.SetAcceptanceRadius(PreferredRange);
			AIController->MoveTo(MoveReq);
		}
		else
		{
			// 타겟이 독구름 안이면 안전 구역 경계까지만 접근
			FVector DirToTarget = (TargetLoc - MyLoc).GetSafeNormal();
			FVector MovePos = MyLoc + DirToTarget * 300.0f;

			if (PoisonZone && !PoisonZone->IsPositionSafe(MovePos))
			{
				AIController->StopMovement();
			}
			else
			{
				AIController->MoveToLocation(MovePos);
			}
		}
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
			
			// 약간의 전진/후진 섞기
			float ForwardBias = FMath::RandRange(-0.5f, 0.5f);
			FVector FinalDir = (StrafeDir + DirToTarget * ForwardBias).GetSafeNormal();
			FVector StrafePos = MyLoc + FinalDir * StrafeRadius;
			
			// 독구름 체크
			if (PoisonZone && !PoisonZone->IsPositionSafe(StrafePos))
			{
				FVector DirToSafe = (PoisonZone->GetActorLocation() - MyLoc).GetSafeNormal();
				StrafePos = MyLoc + (FinalDir * 0.5f + DirToSafe * 0.5f).GetSafeNormal() * StrafeRadius;
			}

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
	}
}

	

void UBTT_MoveToIdealRange::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	// 필요시 정리 작업
}
