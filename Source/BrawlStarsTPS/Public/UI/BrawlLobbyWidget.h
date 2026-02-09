// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlLobbyWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;

/**
 * UBrawlLobbyWidget
 * 
 * 메인 로비 화면 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlLobbyWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	UButton* ButtonPlay;
	
	UPROPERTY(meta=(BindWidget))
	UButton* ButtonSelectBrawler;
	
	UPROPERTY(meta=(BindWidget))
	UButton* ButtonSelectMode;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TextModeName;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TextMapName;

public:
	/** 브롤러 선택 버튼 클릭 시 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Lobby")
	void OnBrawlerSelectClicked();

	/** 모드 선택 버튼 클릭 시 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Lobby")
	void OnModeSelectClicked();

	/** 플레이 버튼 클릭 시 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Lobby")
	void OnPlayClicked();

protected:
	/** 현재 선택된 브롤러 정보 업데이트 (델리게이트 콜백) */
	UFUNCTION()
	void UpdateSelectedBrawlerUI(FName NewRowName);

	/** 현재 선택된 모드 정보 업데이트 (델리게이트 콜백) */
	UFUNCTION()
	void UpdateSelectedModeUIFromRow(FName NewRowName);

	/** 현재 선택된 모드 정보 업데이트 (블루프린트 확장용) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|Lobby")
	void UpdateSelectedModeUI();

protected:
	/** 브롤러 선택 팝업 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> BrawlerSelectWidgetClass;

	/** 게임 모드 선택 팝업 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> GameModeSelectWidgetClass;
};
