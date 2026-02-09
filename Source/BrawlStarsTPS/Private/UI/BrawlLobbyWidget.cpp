// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlLobbyWidget.h"
#include "BrawlGameInstance.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/BrawlLobbyPlayerController.h"
#include "Data/BrawlGameModeData.h"

void UBrawlLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (ButtonPlay)
	{
		ButtonPlay->OnClicked.AddDynamic(this, &UBrawlLobbyWidget::OnPlayClicked);
	}

	if (ButtonSelectBrawler)
	{
		ButtonSelectBrawler->OnClicked.AddDynamic(this, &UBrawlLobbyWidget::OnBrawlerSelectClicked);
	}

	if (ButtonSelectMode)
	{
		ButtonSelectMode->OnClicked.AddDynamic(this, &UBrawlLobbyWidget::OnModeSelectClicked);
	}

	// 델리게이트 바인딩 및 초기 UI 설정
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		// 중복 바인딩 방지를 위해 먼저 제거 시도
		GI->OnBrawlerChanged.RemoveDynamic(this, &UBrawlLobbyWidget::UpdateSelectedBrawlerUI);
		GI->OnBrawlerChanged.AddDynamic(this, &UBrawlLobbyWidget::UpdateSelectedBrawlerUI);

		GI->OnGameModeChanged.RemoveDynamic(this, &UBrawlLobbyWidget::UpdateSelectedModeUIFromRow);
		GI->OnGameModeChanged.AddDynamic(this, &UBrawlLobbyWidget::UpdateSelectedModeUIFromRow);
		
		// 초기값으로 UI 업데이트
		UpdateSelectedBrawlerUI(GI->SelectedBrawlerRowName);
		UpdateSelectedModeUIFromRow(GI->SelectedGameModeRowName);
	}
}

void UBrawlLobbyWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->OnBrawlerChanged.RemoveDynamic(this, &UBrawlLobbyWidget::UpdateSelectedBrawlerUI);
		GI->OnGameModeChanged.RemoveDynamic(this, &UBrawlLobbyWidget::UpdateSelectedModeUIFromRow);
	}
}

void UBrawlLobbyWidget::OnBrawlerSelectClicked()
{
	if (ABrawlLobbyPlayerController* PC = Cast<ABrawlLobbyPlayerController>(GetOwningPlayer()))
	{
		PC->ShowBrawlerSelect();
	}
}

void UBrawlLobbyWidget::OnModeSelectClicked()
{
	if (ABrawlLobbyPlayerController* PC = Cast<ABrawlLobbyPlayerController>(GetOwningPlayer()))
	{
		PC->ShowGameModeSelect();
	}
}

void UBrawlLobbyWidget::OnPlayClicked()
{
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->StartGame();
	}
}

void UBrawlLobbyWidget::UpdateSelectedBrawlerUI(FName NewRowName)
{
	// 브롤러가 변경되었을 때 호출됩니다.
	// 로비 화면의 3D 브롤러 모델을 교체하거나 하는 로직을 이곳에 구현할 수 있습니다.
	// 다만 현재는 BrawlBrawlerPreview에서 처리하고 있습니다.
	UE_LOG(LogTemp, Log, TEXT("UBrawlLobbyWidget::UpdateSelectedBrawlerUI - Selected Brawler changed to: %s"), *NewRowName.ToString());
}

void UBrawlLobbyWidget::UpdateSelectedModeUIFromRow(FName NewRowName)
{
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		if (GI->GameModeDataTable)
		{
			if (FBrawlGameModeData* ModeData = GI->GameModeDataTable->FindRow<FBrawlGameModeData>(NewRowName, TEXT("")))
			{
				ImageModeIcon->SetBrushFromSoftTexture(ModeData->ModeIcon);
				TextModeName->SetText(ModeData->ModeName);
				TextMapName->SetText(ModeData->MapDisplayName);
			}
		}

		UpdateSelectedModeUI();
	}
}



