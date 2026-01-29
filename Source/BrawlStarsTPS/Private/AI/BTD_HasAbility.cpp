// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTD_HasAbility.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"

UBTD_HasAbility::UBTD_HasAbility()
{
	NodeName = TEXT("Has Ability");
}

bool UBTD_HasAbility::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn) return false;

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Pawn);
	if (!ASCInterface) return false;

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC) return false;

	// 태그에 해당하는 어빌리티 스펙 찾기
	TArray<FGameplayAbilitySpec*> Specs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(AbilityTag), Specs);

	for (const FGameplayAbilitySpec* Spec : Specs)
	{
		if (Spec && Spec->Ability)
		{
			// 현재 실행 가능한지 확인 (쿨다운, 코스트, 태그 조건 등)
			if (Spec->Ability->CanActivateAbility(Spec->Handle, ASC->AbilityActorInfo.Get()))
			{
				return true;
			}
		}
	}

	return false;
}
