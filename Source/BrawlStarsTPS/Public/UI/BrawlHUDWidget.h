// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "GameplayEffectTypes.h"
#include "BrawlHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;

// 값이 변경되었을 때 블루프린트로 쏴줄 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

/**
 * UBrawlHUDWidget
 * 
 * 메인 HUD 위젯 클래스입니다.
 * 캐릭터의 체력, 탄환, 궁극기 게이지 등의 변화를 감지하고 이벤트를 발생시킵니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlHUDWidget : public UBrawlUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯 초기화 및 GAS 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void BindAttributeCallbacks(class UAbilitySystemComponent* ASC);

protected:
	// 속성 변경 시 호출될 콜백들
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnAmmoChanged(const FOnAttributeChangeData& Data);
	void OnMaxAmmoChanged(const FOnAttributeChangeData& Data);
	void OnSuperChargeChanged(const FOnAttributeChangeData& Data);
	void OnMaxSuperChargeChanged(const FOnAttributeChangeData& Data);
	void OnHyperChargeChanged(const FOnAttributeChangeData& Data);
	void OnMaxHyperChargeChanged(const FOnAttributeChangeData& Data);

public:
	// 블루프린트에서 바인딩할 UI 요소들
	UPROPERTY(meta=(BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;
	
	// UPROPERTY(meta=(BindWidget))
	// UProgressBar* SuperGauge;
	//
	// UPROPERTY(meta=(BindWidget))
	// UProgressBar* HyperGauge;
	
	// 블루프린트에서 바인딩할 이벤트들
	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI|Attributes")
	FOnAttributeChangedSignature OnHealthChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI|Attributes")
	FOnAttributeChangedSignature OnAmmoChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI|Attributes")
	FOnAttributeChangedSignature OnMaxAmmoChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI|Attributes")
	FOnAttributeChangedSignature OnSuperChargeChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI|Attributes")
	FOnAttributeChangedSignature OnMaxSuperChargeChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI|Attributes")
	FOnAttributeChangedSignature OnHyperChargeChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI|Attributes")
	FOnAttributeChangedSignature OnMaxHyperChargeChangedDelegate;
};
