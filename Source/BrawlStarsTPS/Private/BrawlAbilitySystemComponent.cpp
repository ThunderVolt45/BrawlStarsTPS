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
			UE_LOG(LogTemp, Warning, TEXT("Granting Ability: [%s]"), *AbilityClass->GetName());

			// 1. 입력 태그가 있다면 매핑
			if (BrawlAbility->StartupInputTag.IsValid())
			{
				AbilitySpec.GetDynamicSpecSourceTags().AddTag(BrawlAbility->StartupInputTag);
			}

			// 2. 어빌리티 부여 및 핸들 획득
			const FGameplayAbilitySpecHandle Handle = GiveAbility(AbilitySpec);

			// 3. 입력 태그가 없다면 패시브로 간주하고 즉시 활성화 시도
			// (GA_Reload_Auto 등의 자동 실행을 위해 필요)
			if (!BrawlAbility->StartupInputTag.IsValid())
			{
				TryActivateAbility(Handle);
			}
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
