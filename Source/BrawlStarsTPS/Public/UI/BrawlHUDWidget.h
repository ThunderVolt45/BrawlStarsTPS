// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "GameplayEffectTypes.h"
#include "BrawlHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UBrawlSkillWidget;
class UBrawlGadgetWidget;
class UBrawlSuperWidget;
class UBrawlHyperWidget;
class UPanelWidget; // 추가

// 값이 변경되었을 때 블루프린트로 쏴줄 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

/**
 * UBrawlHUDWidget
 * 
 * 메인 HUD 위젯 클래스입니다.
 * 캐릭터의 사용자의 체력, 탄환, 궁극기 게이지 등의 변화를 감지하고 이벤트를 발생시킵니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlHUDWidget : public UBrawlUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯 초기화 및 GAS 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void BindAttributeCallbacks(class UAbilitySystemComponent* ASC);

	// 게임 모드별 전용 위젯 초기화
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void InitializeGameModeWidget();

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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
	TObjectPtr<UProgressBar> HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> AmmoBar;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> AmmoText;

	// 스킬 위젯들 (WBP_BrawlHUD에서 이름이 일치해야 함)
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBrawlGadgetWidget> Gadget1Widget;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UBrawlGadgetWidget> Gadget2Widget;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBrawlSuperWidget> SuperWidget;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UBrawlHyperWidget> HyperWidget;

	// 게임 남은 시간 (분:초)
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> MatchTimerText;

	// 게임 모드별 추가 위젯을 담을 컨테이너 (CanvasPanel, Overlay 등)
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> GameModeWidgetContainer;
	
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

protected:
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 게임 모드 클래스별 위젯 클래스 매핑
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TMap<TSubclassOf<class AGameModeBase>, TSubclassOf<UUserWidget>> GameModeSpecificWidgets;

	// 현재 활성화된 게임 모드 위젯
	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveGameModeWidget;
};
