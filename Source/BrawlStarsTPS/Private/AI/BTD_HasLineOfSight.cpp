// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTD_HasLineOfSight.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UBTD_HasLineOfSight::UBTD_HasLineOfSight()
{
	NodeName = TEXT("Has Line Of Sight");
}

bool UBTD_HasLineOfSight::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
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

	// 블랙보드에서 타겟 가져오기
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!TargetActor)
	{
		return false;
	}

	// 트레이스 시작점: 캐릭터의 눈 위치 (없으면 Actor Location + UpVector * BaseEyeHeight)
	FVector StartLocation = Pawn->GetPawnViewLocation();
	FVector EndLocation = TargetActor->GetActorLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);
	// 타겟도 무시 목록에 넣고, Hit 결과가 없는 것을 확인할 수도 있지만,
	// 여기서는 타겟까지의 라인 트레이스이므로 타겟에 닿으면 성공, 그 전에 다른 것에 닿으면 실패로 처리하는 것이 정확함.
	// 하지만 LineTraceSingleByChannel은 첫 번째 Blocking Hit를 반환하므로, 
	// 타겟을 Ignore하면 타겟 뒤의 벽을 칠 수 있음. 따라서 타겟은 Ignore하지 않음.

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		TraceChannel,
		Params
	);

	bool bHasLineOfSight = false;

	if (bHit)
	{
		// 무언가에 맞았을 때, 그것이 타겟이라면 시야 확보 성공
		if (HitResult.GetActor() == TargetActor)
		{
			bHasLineOfSight = true;
		}
		else
		{
			// 타겟이 아닌 다른 물체에 가려짐
			bHasLineOfSight = false;
		}
	}
	else
	{
		// 아무것도 맞지 않음 -> 시야가 뚫려있다고 판단할 수도 있지만,
		// 타겟이 범위 내에 있는데 LineTrace가 안 닿을 수는 없음 (EndLocation이 TargetLocation이므로).
		// 만약 도달하기 전에 최대 거리 제한 등으로 끊긴다면 여기서 false가 될 것임.
		// 하지만 LineTraceSingleByChannel은 거리 제한이 Start-End 길이만큼임.
		// 따라서 bHit가 false라는 것은... 
		// 1. 타겟이 Collision이 없는 경우 (Overlap Only 등)
		// 2. 완전히 비어있는 공간
		// 이 경우 시야가 확보된 것으로 간주하는 것이 타당함.
		bHasLineOfSight = true;
	}

	// 디버그 드로잉
	if (bDrawDebug)
	{
		FColor DrawColor = bHasLineOfSight ? FColor::Green : FColor::Red;
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, DrawColor, false, 0.5f);
		if (bHit && HitResult.GetActor() != TargetActor)
		{
			DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Red, false, 0.5f);
		}
	}

	return bHasLineOfSight;
}
