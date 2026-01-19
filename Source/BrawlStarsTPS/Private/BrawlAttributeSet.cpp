// Fill out your copyright notice in the Description page of Project Settings.

#include "BrawlAttributeSet.h"
#include "GameplayEffectExtension.h"

UBrawlAttributeSet::UBrawlAttributeSet()
{
}

void UBrawlAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 각 수치들이 변경될 때 자동으로 최소/최대 값 이내로 제한
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAmmo());
	}
}

void UBrawlAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 각 수치들이 변경될 때 자동으로 최소/최대 값 이내로 제한
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetAmmoAttribute())
	{
		SetAmmo(FMath::Clamp(GetAmmo(), 0.0f, GetMaxAmmo()));
	}
}
