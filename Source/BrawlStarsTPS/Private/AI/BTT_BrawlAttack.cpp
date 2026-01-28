// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_BrawlAttack.h"
#include "AI/BrawlAIController.h"
#include "BrawlCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrawlAbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTT_BrawlAttack::UBTT_BrawlAttack()
{
	NodeName = TEXT("Brawl Attack");
}

EBTNodeResult::Type UBTT_BrawlAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABrawlAIController* AIController = Cast<ABrawlAIController>(OwnerComp.GetAIOwner());
	ABrawlCharacter* MyPawn = AIController ? Cast<ABrawlCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AIController || !MyPawn || !Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	// 1. 타겟 확인 및 조준
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (TargetActor)
	{
		// 타겟 방향 벡터 계산
		FVector StartLocation = MyPawn->GetActorLocation();
		
		// 총구 위치가 있다면 거기서부터 계산하는 게 좋지만, 일단 ActorLocation 기준으로 계산
		FVector TargetLocation = TargetActor->GetActorLocation();
		
		FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
		FRotator NewControlRot = Direction.Rotation();
		NewControlRot.Pitch += AimPitchOffset;

		// 1. 컨트롤러 회전 (발사체 방향 결정)
		AIController->SetFocus(TargetActor);
		AIController->SetControlRotation(NewControlRot);

		// 2. 캐릭터 몸체 회전 (Yaw 값만 적용하여 몸을 돌림)
		FRotator NewActorRot = NewControlRot;
		NewActorRot.Pitch = 0.0f;
		NewActorRot.Roll = 0.0f;
		MyPawn->SetActorRotation(NewActorRot);
	}

	// 2. 어빌리티 발동 시도
	UBrawlAbilitySystemComponent* ASC = MyPawn->GetBrawlAbilitySystemComponent();
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(AbilityTag);

	if (ASC->TryActivateAbilitiesByTag(TagContainer))
	{
		// 발동 성공 -> 딜레이 후 종료
		if (PostAttackDelay > 0.0f)
		{
			FTimerHandle TimerHandle;
			OwnerComp.GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, &OwnerComp]()
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}, PostAttackDelay, false);

			return EBTNodeResult::InProgress;
		}
		else
		{
			return EBTNodeResult::Succeeded;
		}
	}
	else
	{
		// 발동 실패 (쿨다운, 탄약 부족 등)
		return EBTNodeResult::Failed;
	}
}
