// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"

ABrawlLobbyPlayerController::ABrawlLobbyPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ABrawlLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 커서 표시 및 입력 모드 설정
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	// 1. 배경 위젯 생성 및 표시 (가장 먼저 추가하여 밑에 깔리게 함)
	if (BackgroundWidgetClass)
	{
		BackgroundWidget = CreateWidget<UUserWidget>(this, BackgroundWidgetClass);
		if (BackgroundWidget)
		{
			// ZOrder를 -100 정도로 낮게 주어 확실히 뒤에 배치 (동일 레이어라면 추가 순서대로 쌓임)
			BackgroundWidget->AddToViewport(-100);
		}
	}

	// 2. 로비 메인 위젯 생성 및 표시
	if (LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport(0);
		}
	}
}