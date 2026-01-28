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
		// 타겟을 바라보도록 회전 (즉시)
		FVector Direction = TargetActor->GetActorLocation() - MyPawn->GetActorLocation();
		Direction.Z = 0.0f;
		FRotator NewRotation = Direction.Rotation();
		MyPawn->SetActorRotation(NewRotation);
		AIController->SetControlRotation(NewRotation); // 컨트롤러 회전도 맞춰줘야 발사체가 올바르게 나감
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
