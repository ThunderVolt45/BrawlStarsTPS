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

	// 1. 검사 방향 결정
	// 현재 이동 중(속도 있음)이면 이동 방향을, 멈춰있으면 전방을 사용
	FVector CheckDir = Pawn->GetVelocity().GetSafeNormal();
	if (CheckDir.IsNearlyZero())
	{
		CheckDir = Pawn->GetActorForwardVector();
	}

	// 2. 트레이스 시작/끝 지점 계산
	FVector Start = Pawn->GetActorLocation();
	FVector End = Start + (CheckDir * CheckDistance);

	// 자기 자신 무시 설정
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);

	// 3. 구체 트레이스 (Sphere Trace) 실행
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	// 4. 디버그 드로잉
	if (bDrawDebug)
	{
		FColor DrawColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugSphere(GetWorld(), Start, TraceRadius, 12, DrawColor, false, 0.5f);
		DrawDebugLine(GetWorld(), Start, End, DrawColor, false, 0.5f);
		
		if (bHit)
		{
			DrawDebugSphere(GetWorld(), HitResult.Location, 10.0f, 12, FColor::Blue, false, 0.5f);
		}
	}

	return bHit;
}
