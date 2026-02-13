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

	if (!IsRunningDedicatedServer())
	{
		// 로딩 화면 리소스 프리로드
		LoadingBackgroundMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, 
			TEXT("/Game/Materials/M_ScrollingBackground.M_ScrollingBackground")));
		
		LoadingCenterTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, 
			TEXT("/Game/UI/Textures/2239_10x.2239_10x")));
		
		LoadingWingTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, 
			TEXT("/Game/UI/Textures/2238_10x.2238_10x")));
		
		LoadingFillerTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, 
			TEXT("/Game/Textures/WhiteDot.WhiteDot")));

		// 텍스처 리소스가 즉시 사용 가능하도록 보장
		if (LoadingCenterTexture) LoadingCenterTexture->UpdateResource();
		if (LoadingWingTexture) LoadingWingTexture->UpdateResource();
		if (LoadingFillerTexture) LoadingFillerTexture->UpdateResource();

		// 초기 로딩 화면 설정 (게임 시작 시 적용됨)
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.MinimumLoadingScreenDisplayTime = 2.0f; // 초기 시작은 조금 더 길게
		LoadingScreen.WidgetLoadingScreen = SNew(SBrawlLoadingScreen).GameInstance(this);
		
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
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
		LoadingScreen.WidgetLoadingScreen = SNew(SBrawlLoadingScreen).GameInstance(this);
		
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
