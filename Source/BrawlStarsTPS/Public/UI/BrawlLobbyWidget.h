// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlLobbyWidget.generated.h"

/**
 * UBrawlLobbyWidget
 * 
 * 메인 로비 화면 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlLobbyWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

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
	/** 현재 선택된 브롤러 정보 업데이트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|Lobby")
	void UpdateSelectedBrawlerUI();

	/** 현재 선택된 모드 정보 업데이트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|Lobby")
	void UpdateSelectedModeUI();
};
