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
	/** 1단계: 정보 설정 */
	void SetupMatchInfo(FText ModeName, FText Description);

	/** 3단계: 텍스트 숨기기 */
	void HideInfoText();

	/** 4단계: "START" 로고 연출 */
	UFUNCTION(BlueprintNativeEvent, Category = "Brawl|UI")
	void PlayStartAnimation();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_ModeName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Description;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_StartLogo;

	/** 위젯 블루프린트에서 만든 'StartLogoAnim' 애니메이션과 자동 바인딩 */
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> StartLogoAnim;
};