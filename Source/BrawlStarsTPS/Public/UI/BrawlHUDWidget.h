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
class UHorizontalBox; // 추가
class UBrawlAmmoSlotWidget; // 추가

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

	// 캐릭터 데이터를 기반으로 스킬 아이콘 등을 초기화
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void InitializeBrawlerUI(class ABrawlCharacter* Character);

	// 게임 모드별 전용 위젯 초기화
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void InitializeGameModeWidget();

	// 탄약 없음 경고 애니메이션 재생 (떨림)
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void PlayNoAmmoAnimation();

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

	// 탄약 UI 업데이트 (슬롯 방식)
	void UpdateAmmoSlots(float CurrentAmmo, float MaxAmmo);

	// 현재 재장전 어빌리티로부터 진행도 가져오기
	float GetReloadProgress() const;

public:
	// 블루프린트에서 바인딩할 UI 요소들
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	// 탄약 슬롯들을 담을 가로 박스 (필수 바인딩)
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> AmmoSlotContainer;

	// 탄약 슬롯 위젯 클래스 (BP에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI")
	TSubclassOf<UBrawlAmmoSlotWidget> AmmoSlotClass;

	// 생성된 탄약 슬롯 위젯 목록
	UPROPERTY()
	TArray<TObjectPtr<UBrawlAmmoSlotWidget>> AmmoSlotWidgets;

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

	// 조준 보조 리틱클 (원형 감지 영역)
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<class UImage> ReticleCircle;

	// 조준 보조 감지 반경 (시각화용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Aim")
	float ReticleCircleRadius = 100.0f;

	// 현재 조준된 대상을 따라다니는 보조 이미지
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<class UImage> TargetIndicator;
	
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
