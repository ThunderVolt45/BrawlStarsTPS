// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTS_UpdateSelfStatus.h"
#include "AIController.h"
#include "BrawlCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"

UBTS_UpdateSelfStatus::UBTS_UpdateSelfStatus()
{
	NodeName = TEXT("Update Self Status");
	
	// Float 키만 선택 가능하도록 필터링
	HealthRatioKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_UpdateSelfStatus, HealthRatioKey));
}

void UBTS_UpdateSelfStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	ABrawlCharacter* MyPawn = Cast<ABrawlCharacter>(AIController->GetPawn());
	if (!MyPawn) return;

	UAbilitySystemComponent* ASC = MyPawn->GetAbilitySystemComponent();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (ASC && Blackboard)
	{
		bool bFoundHealth = false;
		float Health = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetHealthAttribute(), bFoundHealth);
		float MaxHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxHealthAttribute(), bFoundHealth);

		if (bFoundHealth && MaxHealth > 0.0f)
		{
			float HealthRatio = FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
			
			// 블랙보드 업데이트
			Blackboard->SetValueAsFloat(HealthRatioKey.SelectedKeyName, HealthRatio);
		}
	}
}
