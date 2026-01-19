// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BrawlAbilitySystemComponent.generated.h"

class UBrawlGameplayAbility;

/**
 * UBrawlAbilitySystemComponent
 *
 * 프로젝트 전용 Ability System Component 클래스입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	UBrawlAbilitySystemComponent();

	bool bCharacterAbilitiesGiven = false;

	// 캐릭터 초기 어빌리티 부여
	void AddCharacterAbilities(const TArray<TSubclassOf<UBrawlGameplayAbility>>& StartupAbilities);

	// 입력 태그 처리
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

protected:
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
};
