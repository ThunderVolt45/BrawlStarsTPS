#include "AI/BTT_Jump.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTT_Jump::UBTT_Jump()
{
	NodeName = TEXT("Simple Jump");
	bNotifyTick = true;
	
	// 블랙보드 키 필터 (Object or Vector)
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_Jump, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_Jump, TargetKey));
}

EBTNodeResult::Type UBTT_Jump::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ACharacter* MyCharacter = AIController ? Cast<ACharacter>(AIController->GetPawn()) : nullptr;

	if (!MyCharacter)
	{
		return EBTNodeResult::Failed;
	}
	
	// 0. 만약 Z축 속도가 0이 아니라면 (떨어지거나 점프 중이었다면) 취소
	if (!FMath::IsNearlyZero(MyCharacter->GetVelocity().Z))
	{
		return EBTNodeResult::Failed;
	}

	// 1. 점프 방향 계산
	FVector JumpDir = GetJumpDirection(OwnerComp, MyCharacter);

	// 2. 최종 발사 벡터 계산
	FVector LaunchVelocity = (JumpDir * JumpForwardSpeed) + FVector(0.0f, 0.0f, JumpZ);

	// 3. 캐릭터 발사 (Launch)
	MyCharacter->LaunchCharacter(LaunchVelocity, true, false);

	// 점프 방향으로 즉시 회전
	// if (!JumpDir.IsNearlyZero())
	// {
	// 	MyCharacter->SetActorRotation(JumpDir.Rotation());
	// }
	
	return EBTNodeResult::Succeeded;
}

void UBTT_Jump::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}

FVector UBTT_Jump::GetJumpDirection(UBehaviorTreeComponent& OwnerComp, ACharacter* MyCharacter)
{
	FVector JumpDir = MyCharacter->GetActorForwardVector();

	if (!TargetKey.IsNone())
	{
		if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
		{
			FVector TargetLocation = FVector::ZeroVector;
			bool bHasTarget = false;

			if (TargetKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
			{
				UObject* KeyValue = Blackboard->GetValueAsObject(TargetKey.SelectedKeyName);
				if (AActor* TargetActor = Cast<AActor>(KeyValue))
				{
					TargetLocation = TargetActor->GetActorLocation();
					bHasTarget = true;
				}
			}
			else if (TargetKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
			{
				TargetLocation = Blackboard->GetValueAsVector(TargetKey.SelectedKeyName);
				bHasTarget = true;
			}

			if (bHasTarget)
			{
				JumpDir = (TargetLocation - MyCharacter->GetActorLocation());
				JumpDir.Z = 0.0f;
				JumpDir.Normalize();
			}
		}
	}

	if (bJumpAway)
	{
		JumpDir *= -1.0f;
	}

	return JumpDir;
}

