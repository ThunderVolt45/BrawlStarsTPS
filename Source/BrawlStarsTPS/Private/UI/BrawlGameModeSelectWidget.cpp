// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlGameModeSelectWidget.h"
#include "Components/Button.h"
#include "UI/BrawlLobbyPlayerController.h"

void UBrawlGameModeSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ButtonExit)
	{
		ButtonExit->OnClicked.AddDynamic(this, &UBrawlGameModeSelectWidget::OnExitClicked);
	}
}

void UBrawlGameModeSelectWidget::OnExitClicked()
{
	if (ABrawlLobbyPlayerController* PC = Cast<ABrawlLobbyPlayerController>(GetOwningPlayer()))
	{
		PC->ShowLobby();
	}
}
