// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlGameModeButtonWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "BrawlGameInstance.h"
#include "UI/BrawlLobbyPlayerController.h"

void UBrawlGameModeButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonSelect)
	{
		ButtonSelect->OnClicked.AddDynamic(this, &UBrawlGameModeButtonWidget::OnButtonClicked);
	}

	RefreshWidget();
}

void UBrawlGameModeButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshWidget();
}

void UBrawlGameModeButtonWidget::InitializeButton(FName InRowId, FText InDisplayName, TSoftObjectPtr<UTexture2D> InIcon)
{
	ModeRowId = InRowId;
	ModeName = InDisplayName;
	ModeIcon = InIcon;

	RefreshWidget();
}

void UBrawlGameModeButtonWidget::RefreshWidget()
{
	if (TextName)
	{
		TextName->SetText(ModeName);
	}

	if (ImageIcon && !ModeIcon.IsNull())
	{
		if (UTexture2D* IconTexture = ModeIcon.LoadSynchronous())
		{
			ImageIcon->SetBrushFromTexture(IconTexture);
		}
	}
}

void UBrawlGameModeButtonWidget::OnButtonClicked()
{
	PlayClickSFX();

	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->SetSelectedGameMode(ModeRowId);
		
		UE_LOG(LogTemp, Log, TEXT("Game Mode Selected: %s"), *ModeRowId.ToString());

		if (ABrawlLobbyPlayerController* PC = Cast<ABrawlLobbyPlayerController>(GetOwningPlayer()))
		{
			PC->ShowLobby();
		}
	}
}
