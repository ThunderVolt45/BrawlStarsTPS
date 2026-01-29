// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTD_IsObstacleDestructible.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Environment/BrawlDestructibleInterface.h"
#include "DrawDebugHelpers.h"

UBTD_IsObstacleDestructible::UBTD_IsObstacleDestructible()
{
	NodeName = TEXT("Is Obstacle Destructible");

	// 블랙보드 키 필터 설정 (Actor 또는 Vector)
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTD_IsObstacleDestructible, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTD_IsObstacleDestructible, TargetKey));
}

bool UBTD_IsObstacleDestructible::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn) return false;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return false;

	// 1. 타겟 위치 파악
	FVector TargetLocation = FVector::ZeroVector;
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
	
	if (TargetActor)
	{
		TargetLocation = TargetActor->GetActorLocation();
	}
	else
	{
		// 액터가 없으면 벡터 키 확인
		TargetLocation = Blackboard->GetValueAsVector(TargetKey.SelectedKeyName);
	}

	if (TargetLocation.IsNearlyZero()) return false;

	// 2. 라인 트레이스로 장애물 확인
	FVector Start = Pawn->GetActorLocation();
	FVector End = TargetLocation;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);
	if (TargetActor) Params.AddIgnoredActor(TargetActor);

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel, Params);

	if (bHit && HitResult.GetActor())
	{
		// 3. 인터페이스 구현 여부 확인
		if (HitResult.GetActor()->Implements<UBrawlDestructibleInterface>())
		{
			if (bDrawDebug)
			{
				DrawDebugLine(GetWorld(), Start, HitResult.Location, FColor::Red, false, 0.5f);
				DrawDebugBox(GetWorld(), HitResult.GetActor()->GetActorLocation(), FVector(50.0f), FColor::Red, false, 0.5f);
			}
			return true;
		}
	}

	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.5f);
	}

	return false;
}
