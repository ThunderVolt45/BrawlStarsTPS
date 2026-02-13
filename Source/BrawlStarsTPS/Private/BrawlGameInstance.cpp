// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Data/BrawlGameModeData.h"
#include "MoviePlayer.h"
#include "UI/SBrawlLoadingScreen.h"

UBrawlGameInstance::UBrawlGameInstance()
{
}

void UBrawlGameInstance::Init()
{
	Super::Init();
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

	ShowLoadingScreen();
	UGameplayStatics::OpenLevel(this, SelectedMapName);
}

void UBrawlGameInstance::ShowLoadingScreen()
{
	if (!IsRunningDedicatedServer())
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true; // 로드 완료 시 자동 종료
		LoadingScreen.MinimumLoadingScreenDisplayTime = 1.0f; // 최소 1초 노출
		LoadingScreen.WidgetLoadingScreen = SNew(SBrawlLoadingScreen);
		
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
		
		// 로딩 화면 재생 시작 (OpenLevel이 호출되면 MoviePlayer가 이를 감지하여 UI를 유지함)
		GetMoviePlayer()->PlayMovie();
	}
}

void UBrawlGameInstance::HideLoadingScreen()
{
	if (!IsRunningDedicatedServer())
	{
		GetMoviePlayer()->StopMovie();
	}
}
