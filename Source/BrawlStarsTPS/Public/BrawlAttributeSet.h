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
	
	// 재장전 속도
	UPROPERTY(BlueprintReadOnly, Category= "Attributes")
	FGameplayAttributeData ReloadSpeed;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, ReloadSpeed);
	
	// 기본 공격 데미지
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Damage")
	FGameplayAttributeData AttackDamage;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, AttackDamage);

	// 가젯 데미지
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Damage")
	FGameplayAttributeData GadgetDamage;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, GadgetDamage);

	// 가젯 쿨다운 (초)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Cooldown")
	FGameplayAttributeData GadgetCooldown;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, GadgetCooldown);

	// 궁극기 게이지 (SuperCharge)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData SuperCharge;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, SuperCharge);

	// 최대 궁극기 게이지
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxSuperCharge;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, MaxSuperCharge);
	
	// 궁극기 소모량 (게이지)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Cost")
	FGameplayAttributeData SuperCost;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, SuperCost);
	
	// 궁극기 데미지
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Damage")
	FGameplayAttributeData SuperDamage;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, SuperDamage);

	// 평타 1회 명중 시 궁극기 충전량
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData SuperChargePerHit;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, SuperChargePerHit);

	// 하이퍼차지 게이지 (공격 적중 시 충전)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Hyper")
	FGameplayAttributeData HyperCharge;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, HyperCharge);

	// 최대 하이퍼차지 게이지
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Hyper")
	FGameplayAttributeData MaxHyperCharge;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, MaxHyperCharge);

	// 공격 적중 시 하이퍼차지 충전량 (Table에서 가져옴)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Hyper")
	FGameplayAttributeData HyperChargePerHit;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, HyperChargePerHit);

	// 이동 속도 (Movement Speed)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, MovementSpeed);

	// 데미지 감소율 (Damage Reduction) - 0.0 ~ 1.0 (예: 0.3 = 30% 감소)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats")
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, DamageReduction);

	// 메타 속성: 데미지 처리를 위한 임시 값 (서버에서만 유효)
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UBrawlAttributeSet, IncomingDamage);

	//~UAttributeSet interface
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
private:
	void OnGetIncomingDamage(const FGameplayEffectModCallbackData& Data);
};
