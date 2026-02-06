// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlBrawlerButton.generated.h"

class UImage;
class UTextBlock;
class UButton;

/**
 * UBrawlBrawlerButton
 * 
 * 브롤러 선택 리스트에서 개별 브롤러를 나타내는 버튼 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlBrawlerButton : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	/** 위젯 초기화 및 데이터 설정 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void InitializeButton(FName InRowId, FText InDisplayName, TSoftObjectPtr<UTexture2D> InIcon);

	/** UI 요소들의 스타일 및 텍스트를 현재 변수값에 맞춰 갱신 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void RefreshWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	/** 버튼 클릭 핸들러 */
	UFUNCTION()
	void OnButtonClicked();

protected:
	/** 브롤러 ID (실제 로직에서 사용될 값) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI", meta = (ExposeOnSpawn = "true"))
	FName BrawlerRowId;

	/** 화면에 표시될 브롤러 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI", meta = (ExposeOnSpawn = "true"))
	FText BrawlerName;

	/** 화면에 표시될 브롤러 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI", meta = (ExposeOnSpawn = "true"))
	TSoftObjectPtr<UTexture2D> BrawlerIcon;

	/** 아이콘 이미지의 좌표 오프셋 (Render Transform) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI")
	FVector2D IconTranslation = FVector2D::ZeroVector;

	/** 아이콘 이미지의 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI")
	FVector2D IconSize = FVector2D(100.0f, 100.0f);

	/** 배경 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI")
	FLinearColor BackgroundColor = FLinearColor::White;

	/** UI 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageBackground;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonSelect;
};
