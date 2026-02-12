// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlGameModeButton.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "BrawlGameInstance.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "UI/BrawlLobbyPlayerController.h"

void UBrawlGameModeButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonSelect)
	{
		ButtonSelect->OnClicked.AddDynamic(this, &UBrawlGameModeButton::OnButtonClicked);
	}

	RefreshWidget();
}

void UBrawlGameModeButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshWidget();
}

void UBrawlGameModeButton::InitializeButton(FName InRowId, FText InModeName, TSoftObjectPtr<UTexture2D> InModeIcon, FText InMapName, TSoftObjectPtr<UTexture2D> InThemeIcon, FVector2D InImageIconSize)
{
	ModeRowId = InRowId;
	ModeName = InModeName;
	ModeIcon = InModeIcon;
	MapDisplayName = InMapName;
	ThemeIcon = InThemeIcon;

	RefreshWidget();
}

void UBrawlGameModeButton::RefreshWidget()
{
	if (ImageBackground)
	{
		ImageBackground->SetColorAndOpacity(BackgroundColor);
	}

	if (TextModeName)
	{
		TextModeName->SetText(ModeName);
	}

	if (TextMapName)
	{
		TextMapName->SetText(MapDisplayName);
	}

	if (ImageIcon)
	{
		if (!ModeIcon.IsNull())
		{
			if (UTexture2D* IconTexture = ModeIcon.LoadSynchronous())
			{
				ImageIcon->SetBrushFromTexture(IconTexture);
			}
		}
	}

	if (ImageTheme)
	{
		if (!ThemeIcon.IsNull())
		{
			if (UTexture2D* ThemeTexture = ThemeIcon.LoadSynchronous())
			{
				ImageTheme->SetBrushFromTexture(ThemeTexture);
			}
		}
	}
}

void UBrawlGameModeButton::OnButtonClicked()
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
