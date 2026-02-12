// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlGameModeButton.generated.h"

class UImage;
class UTextBlock;
class UButton;

/**
 * UBrawlGameModeButton
 * 
 * 게임 모드 선택 리스트에서 개별 모드를 나타내는 버튼 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameModeButton : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	/** 위젯 초기화 및 데이터 설정 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void InitializeButton(FName InRowId, FText InModeName, TSoftObjectPtr<UTexture2D> InModeIcon, FText InMapName, TSoftObjectPtr<UTexture2D> InThemeIcon, FVector2D InImageIconSize = FVector2D(64.0f, 64.0f));

	/** UI 요소들을 현재 변수값에 맞춰 갱신 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void RefreshWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	/** 버튼 클릭 핸들러 */
	UFUNCTION()
	void OnButtonClicked();

protected:
	/** 게임 모드 ID (DataTable RowName) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI", meta = (ExposeOnSpawn = "true"))
	FName ModeRowId;

	/** 화면에 표시될 모드 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI", meta = (ExposeOnSpawn = "true"))
	FText ModeName;

	/** 화면에 표시될 모드 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI", meta = (ExposeOnSpawn = "true"))
	TSoftObjectPtr<UTexture2D> ModeIcon;

	/** 화면에 표시될 맵 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI", meta = (ExposeOnSpawn = "true"))
	FText MapDisplayName;

	/** 테마 이미지 (배경 등에 쓰일 이미지) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI", meta = (ExposeOnSpawn = "true"))
	TSoftObjectPtr<UTexture2D> ThemeIcon;

	/** 배경 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|UI")
	FLinearColor BackgroundColor = FLinearColor::White;

	/** UI 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageBackground;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageTheme;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextModeName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextMapName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonSelect;
};
