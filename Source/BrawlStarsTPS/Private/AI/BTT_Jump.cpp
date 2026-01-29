// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_Jump.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTT_Jump::UBTT_Jump()
{
	NodeName = TEXT("Jump");
	bNotifyTick = true; // 점프가 끝날 때까지 기다리려면 Tick이 필요할 수 있음
}

EBTNodeResult::Type UBTT_Jump::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* MyCharacter = Cast<ACharacter>(AIController->GetPawn());
	if (!MyCharacter)
	{
		return EBTNodeResult::Failed;
	}

	// 점프 실행
	MyCharacter->Jump();
	
	// 점프 시작 후 성공 반환 (즉발 행동)
	// 만약 체공 시간 동안 대기하고 싶다면 InProgress를 반환하고 Tick에서 검사해야 함.
	// 여기서는 즉시 성공 처리하고 다음 노드로 넘김 (이동 중에 점프하는 등 병렬 처리에 유리)
	return EBTNodeResult::Succeeded;
}

void UBTT_Jump::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 현재는 즉시 성공(Succeeded) 반환하므로 Tick은 호출되지 않음.
	// 추후 착지까지 대기하려면 ExecuteTask에서 InProgress 반환 후 여기서 IsFalling() 체크.
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}
