// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTS_RotateToTarget.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UBTS_RotateToTarget::UBTS_RotateToTarget()
{
	NodeName = TEXT("Rotate To Target Service");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true; // Tick 활성화

	// 타겟 키 필터링 (Actor 또는 Vector 허용)
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_RotateToTarget, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_RotateToTarget, TargetKey));
}

void UBTS_RotateToTarget::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	APawn* MyPawn = AIController->GetPawn();
	ACharacter* MyCharacter = Cast<ACharacter>(MyPawn);

	if (MyCharacter)
	{
		// 서비스 시작 시 회전 동기화 및 모드 설정
		AIController->SetControlRotation(MyCharacter->GetActorRotation());

		if (MyCharacter->GetCharacterMovement())
		{
			// 이동 방향으로 회전하는 기능을 끄고 컨트롤러 회전을 따르도록 설정
			MyCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		}
		
		MyCharacter->bUseControllerRotationYaw = true;
	}
}

void UBTS_RotateToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;
	
	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	// 1. 타겟 위치 확인
	FVector TargetLocation = FVector::ZeroVector;
	bool bTargetValid = false;

	if (Blackboard->IsVectorValueSet(TargetKey.SelectedKeyName))
	{
		TargetLocation = Blackboard->GetValueAsVector(TargetKey.SelectedKeyName);
		bTargetValid = true;
	}
	else if (UObject* KeyObject = Blackboard->GetValueAsObject(TargetKey.SelectedKeyName))
	{
		if (AActor* TargetActor = Cast<AActor>(KeyObject))
		{
			// 타겟의 피벗(발밑)이 아닌 중심점을 바라보도록 보정 (떨림 방지)
			FVector Origin, BoxExtent;
			TargetActor->GetActorBounds(true, Origin, BoxExtent);
			TargetLocation = Origin;
			bTargetValid = true;
		}
	}

	if (!bTargetValid) return;

	// 2. 목표 회전 계산
	FVector MyLocation = MyPawn->GetActorLocation();
	FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();
	
	if (Direction.IsNearlyZero()) return;

	FRotator TargetRotation = Direction.Rotation();
	TargetRotation.Roll = 0.0f;

	// 3. 현재 회전과 비교 (Deadzone 체크)
	FRotator CurrentRotation = AIController->GetControlRotation();
	FRotator DeltaRot = (TargetRotation - CurrentRotation).GetNormalized();

	// 디버그 드로잉
	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), TargetLocation, 20.0f, 12, FColor::Yellow, false, 1.0f);
		DrawDebugLine(GetWorld(), MyLocation, TargetLocation, FColor::Yellow, false, 1.0f);
	}

	// 오차가 허용 범위 이내라면 회전하지 않음 (Jitter 방지)
	if (FMath::Abs(DeltaRot.Yaw) < Precision)
	{
		return;
	}

	// 4. 보간 및 적용
	FRotator NewRotation;
	if (bUseConstantSpeed)
	{
		NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed * 10.0f);
	}
	else
	{
		NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed);
	}

	AIController->SetControlRotation(NewRotation);
}
