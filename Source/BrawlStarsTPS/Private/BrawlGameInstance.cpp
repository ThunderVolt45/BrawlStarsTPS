// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Data/BrawlGameModeData.h"

UBrawlGameInstance::UBrawlGameInstance()
{
}

void UBrawlGameInstance::SetSelectedBrawler(FName NewRowName)
{
	if (SelectedBrawlerRowName != NewRowName)
	{
		SelectedBrawlerRowName = NewRowName;
		OnBrawlerChanged.Broadcast(SelectedBrawlerRowName);
	}
}

void UBrawlGameInstance::SetSelectedGameMode(FName NewRowName)
{
	if (SelectedGameModeRowName != NewRowName)
	{
		SelectedGameModeRowName = NewRowName;
		
		// 데이터 테이블에서 정보 업데이트
		if (GameModeDataTable)
		{
			if (FBrawlGameModeData* ModeData = GameModeDataTable->FindRow<FBrawlGameModeData>(SelectedGameModeRowName, TEXT("")))
			{
				SelectedGameModeType = ModeData->GameModeType;
				SelectedMapName = ModeData->MapName;
			}
		}

		OnGameModeChanged.Broadcast(SelectedGameModeRowName);
	}
}

void UBrawlGameInstance::StartGame()
{
	if (SelectedMapName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlGameInstance: SelectedMapName is None! Cannot start game."));
		return;
	}

	UGameplayStatics::OpenLevel(this, SelectedMapName);
}
