// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.generated.h"

// Attribute 접근자를 위한 매크로
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UBrawlAttributeSet
 *
 * 브롤스타즈 캐릭터의 주요 능력치(체력, 탄환, 궁극기 게이지 등)를 관리합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBrawlAttributeSet();

	// 체력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, Health);

	// 최대 체력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, MaxHealth);

	// 탄환 (Ammo)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Ammo;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, Ammo);

	// 최대 탄환
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxAmmo;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, MaxAmmo);

	// 궁극기 게이지 (SuperCharge)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData SuperCharge;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, SuperCharge);

	//~UAttributeSet interface
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
