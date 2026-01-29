// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTD_IsBlocked.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UBTD_IsBlocked::UBTD_IsBlocked()
{
	NodeName = TEXT("Is Blocked");
}

bool UBTD_IsBlocked::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return false;
	}
	
	// 0. 무작위 확률로 이유없이 그냥 (!) 점프한다 (플레이어처럼)
	if (FMath::RandRange(1, 100) >= (100 - RandomJumpChance))
	{
		return true;
	}

	// 1. 검사 위치 결정
	FVector CheckDir = bCheckForward ? Pawn->GetActorForwardVector() : Pawn->GetActorForwardVector() * -1;

	// 검사 중심점: 캐릭터 위치 + (방향 * 거리)
	FVector CheckLocation = Pawn->GetActorLocation() + (CheckDir * CheckDistance);

	// 2. 오버랩 테스트 (Sphere Overlap)
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn); // 자기 자신 제외

	bool bOverlap = GetWorld()->OverlapBlockingTestByChannel(
		CheckLocation,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	// 3. 디버그 드로잉
	if (bDrawDebug)
	{
		FColor DrawColor = bOverlap ? FColor::Red : FColor::Green;
		DrawDebugSphere(GetWorld(), CheckLocation, TraceRadius, 12, DrawColor, false, 0.5f);
		DrawDebugLine(GetWorld(), Pawn->GetActorLocation(), CheckLocation, FColor::Yellow, false, 0.5f);
	}

	return bOverlap;
}
