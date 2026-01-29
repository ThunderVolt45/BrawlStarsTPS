// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_UseAbility.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UBTT_UseAbility::UBTT_UseAbility()
{
	NodeName = TEXT("Use Ability");
}

EBTNodeResult::Type UBTT_UseAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Pawn);
	if (!ASCInterface) return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC) return EBTNodeResult::Failed;

	// 태그로 어빌리티 발동 시도
	bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));

	if (bActivated)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
