// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Data/BrawlGameModeData.h"
#include "MoviePlayer.h"
#include "UI/SBrawlLoadingScreen.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"

UBrawlGameInstance::UBrawlGameInstance()
{
	// 로딩 화면 리소스 미리 로드 (생성자에서 수행하여 메모리 상주 보장)
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BackgroundMatObj(TEXT("/Game/Materials/M_ScrollingBackground"));
	if (BackgroundMatObj.Succeeded())
	{
		LoadingBackgroundMaterial = BackgroundMatObj.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> CenterTexObj(TEXT("/Game/UI/Textures/2239_10x"));
	if (CenterTexObj.Succeeded())
	{
		LoadingCenterTexture = CenterTexObj.Object;
		LoadingCenterTexture->NeverStream = true;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> WingTexObj(TEXT("/Game/UI/Textures/2238_10x"));
	if (WingTexObj.Succeeded())
	{
		LoadingWingTexture = WingTexObj.Object;
		LoadingWingTexture->NeverStream = true;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> FillerTexObj(TEXT("/Game/Textures/WhiteDot"));
	if (FillerTexObj.Succeeded())
	{
		LoadingFillerTexture = FillerTexObj.Object;
		LoadingFillerTexture->NeverStream = true;
	}
}

void UBrawlGameInstance::Init()
{
	Super::Init();

	if (!IsRunningDedicatedServer())
	{
		// 스트링 테이블 프리로드
		GameModeStringTable = StaticLoadObject(UObject::StaticClass(), nullptr, 
			TEXT("/Game/Data/ST_GameModeInfo.ST_GameModeInfo"));

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
