// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlBrawlerSelectWidget.h"
#include "BrawlGameInstance.h"
#include "Components/Button.h"
#include "UI/BrawlLobbyPlayerController.h"

void UBrawlBrawlerSelectWidget::SelectBrawler(FName BrawlerRowName)
{
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->SetSelectedBrawler(BrawlerRowName);
		
		// 선택 완료 이벤트 호출
		OnBrawlerSelected();

		// 로비로 돌아가기
		if (ABrawlLobbyPlayerController* PC = Cast<ABrawlLobbyPlayerController>(GetOwningPlayer()))
		{
			PC->ShowLobby();
		}
	}
}

void UBrawlBrawlerSelectWidget::OnExitClicked()
{
	if (ABrawlLobbyPlayerController* PC = Cast<ABrawlLobbyPlayerController>(GetOwningPlayer()))
	{
		PC->ShowLobby();
	}
}

void UBrawlBrawlerSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ButtonExit)
	{
		ButtonExit->OnClicked.AddDynamic(this, &UBrawlBrawlerSelectWidget::OnExitClicked);
	}
}
