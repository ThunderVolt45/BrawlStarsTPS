// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlMatchStartWidget.generated.h"

class UTextBlock;
class UImage;
class UWidgetAnimation;

/**
 * UBrawlMatchStartWidget
 * 
 * 게임 시작 시 연출을 담당하는 C++ 기반 위젯 클래스입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlMatchStartWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	// 1단계: 정보 설정
	void SetupMatchInfo(FText ModeName, FText Description);

	// 2단계는 여기서 해당 없음
	
	// 3단계: 텍스트 숨기기
	void HideInfoText();

	/** 4단계: "START" 로고 연출 */
	UFUNCTION(BlueprintNativeEvent, Category = "Brawl|UI")
	void PlayStartAnimation();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextModeName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextDescription;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageStartLogo;

private:
	/** 절차적 애니메이션을 위한 상태 변수 */
	bool bIsAnimatingLogo = false;
	float AnimationProgress = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI|Animation")
	float AnimationDuration = 0.5f;
};