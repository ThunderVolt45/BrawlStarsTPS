// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_RotateToTarget.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTT_RotateToTarget::UBTT_RotateToTarget()
{
	NodeName = TEXT("Rotate To Target");
	bNotifyTick = true;
	
	// 타겟 키 필터링 (Actor 또는 Vector 허용)
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_RotateToTarget, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_RotateToTarget, TargetKey));
}

EBTNodeResult::Type UBTT_RotateToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* MyPawn = AIController ? AIController->GetPawn() : nullptr;

	if (!AIController || !MyPawn)
	{
		return EBTNodeResult::Failed;
	}

	// 1. 회전 설정 변경 (전투 모드: 컨트롤러 회전 따르기)
	ACharacter* MyCharacter = Cast<ACharacter>(MyPawn);
	if (MyCharacter)
	{
		// OrientRotationToMovement를 꺼야 제자리 회전이 자연스러움
		if (MyCharacter->GetCharacterMovement())
		{
			MyCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		}
		
		MyCharacter->bUseControllerRotationYaw = true;
	}

	return EBTNodeResult::InProgress;
}

void UBTT_RotateToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* MyPawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AIController || !MyPawn || !Blackboard)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

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
			TargetLocation = TargetActor->GetActorLocation();
			bTargetValid = true;
		}
	}

	if (!bTargetValid)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 2. 목표 회전 계산
	FVector MyLocation = MyPawn->GetActorLocation();
	FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();
	
	FRotator TargetRotation = Direction.Rotation();
	TargetRotation.Pitch += PitchOffset; // Pitch Offset 적용
	
	// Roll은 0으로 고정
	TargetRotation.Roll = 0.0f;

	// 3. 현재 회전에서 부드럽게 보간
	FRotator CurrentRotation = AIController->GetControlRotation();
	FRotator NewRotation;

	if (bUseConstantSpeed)
	{
		// 등속 회전 (일정한 속도)
		NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed * 10.0f); // Constant는 속도 단위가 다르므로 보정 필요할 수 있음
	}
	else
	{
		// 부드러운 감속 회전 (기본)
		NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed);
	}

	AIController->SetControlRotation(NewRotation);

	// 4. 목표 도달 확인 (Yaw와 Pitch 차이가 오차 범위 내인지)
	// Pitch는 Offset 때문에 정확히 안 맞을 수도 있으니, 주로 Yaw를 체크하거나 전체 Delta 체크
	// 여기서는 전체 회전 차이를 체크하되, Roll은 무시
	float AngleDiff = FMath::Abs((NewRotation - TargetRotation).GetNormalized().Yaw);
	float PitchDiff = FMath::Abs((NewRotation - TargetRotation).GetNormalized().Pitch);

	if (AngleDiff < Precision && PitchDiff < Precision)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
