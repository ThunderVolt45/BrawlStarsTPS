// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlLobbyWidget.h"
#include "BrawlGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UBrawlLobbyWidget::OnBrawlerSelectClicked()
{
	// 브롤러 선택창 UI 오픈 로직 (Blueprint에서 주로 처리)
}

void UBrawlLobbyWidget::OnModeSelectClicked()
{
	// 모드 선택창 UI 오픈 로직 (Blueprint에서 주로 처리)
}

void UBrawlLobbyWidget::OnPlayClicked()
{
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->StartGame();
	}
}
