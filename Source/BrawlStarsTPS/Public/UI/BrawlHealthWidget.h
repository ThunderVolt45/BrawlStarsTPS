// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "GameplayEffectTypes.h"
#include "BrawlHealthWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UAbilitySystemComponent;

/**
 * UBrawlHealthWidget
 * 
 * 캐릭터 머리 위에 표시되는 체력바 위젯 클래스
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlHealthWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	// ASC와 연동하여 체력 변화 감지 시작
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	// 체력 변경 시 호출될 이벤트
	UFUNCTION()
	void OnHealthChanged(float NewValue, float MaxValue);

	// 팀 색상 등을 변경하기 위한 이벤트
	UFUNCTION()
	void OnTeamColorChanged(bool bIsEnemy);

private:
	// GAS 어트리뷰트 변경 콜백
	void HealthChanged(const FOnAttributeChangeData& Data);
	void MaxHealthChanged(const FOnAttributeChangeData& Data);
	void PowerCubeCountChanged(const FOnAttributeChangeData& Data);

	/** 현상금 변경 콜백 (PlayerState로부터 호출) */
	UFUNCTION()
	void OnBountyChanged(int32 NewBounty);

	/** 타이 브레이커 상태 변경 콜백 */
	UFUNCTION()
	void OnTieBreakerStateChanged(bool bHasTieBreaker);

	/** UI 갱신 헬퍼 함수 */
	void UpdatePowerCubeDisplay(float NewCount);
	void UpdateBountyDisplay(int32 NewBounty);
	void UpdateTieBreakerDisplay(bool bHasTieBreaker);

	/** 초기 팀 설정 및 PlayerState 연결 */
	void SetupPlayerStateBindings();

public:
	// 블루프린트에서 바인딩할 UI 요소들
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	// 파워 큐브 개수 텍스트 (아이콘 옆에 표시 권장)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PowerCubeText;

	// 파워 큐브 아이콘
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> PowerCubeIcon;

	// 현상금 별 개수 텍스트
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BountyText;

	// 현상금 아이콘 (별)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> BountyIcon;

	// 타이 브레이커 아이콘
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> TieBreakerIcon;
	
	// 체력 바 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	FLinearColor HealthBarColor = FLinearColor::Green;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	FLinearColor EnemyHealthBarColor = FLinearColor::Red;
	
private:
	// 현재 값 캐싱
	float CurrentHealth = 0.0f;
	float CurrentMaxHealth = 0.0f;

	/** 재시도 횟수 관리 */
	int32 PS_RetryCount = 0;

	/** 이 위젯이 표시하고 있는 대상 캐릭터 */
	UPROPERTY()
	TWeakObjectPtr<class ABrawlCharacter> TargetCharacter;
};
