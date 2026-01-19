// Fill out your copyright notice in the Description page of Project Settings.

#include "BrawlAbilitySystemComponent.h"
#include "Abilities/BrawlGameplayAbility.h"

UBrawlAbilitySystemComponent::UBrawlAbilitySystemComponent()
{
}

void UBrawlAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UBrawlGameplayAbility>>& StartupAbilities)
{
	if (bCharacterAbilitiesGiven) return;

	// 서버 권한 확인 (서버에서만 어빌리티 부여)
	if (!GetOwner()->HasAuthority()) return;

	for (const TSubclassOf<UBrawlGameplayAbility>& AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UBrawlGameplayAbility* BrawlAbility = Cast<UBrawlGameplayAbility>(AbilitySpec.Ability))
		{
			// 어빌리티의 InputTag를 Spec의 DynamicTags에 추가하여 입력과 매핑
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(BrawlAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
	bCharacterAbilitiesGiven = true;
}

void UBrawlAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(Spec);
			if (!Spec.IsActive())
			{
				TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void UBrawlAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(Spec);
		}
	}
}

void UBrawlAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);
}

void UBrawlAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);
}
