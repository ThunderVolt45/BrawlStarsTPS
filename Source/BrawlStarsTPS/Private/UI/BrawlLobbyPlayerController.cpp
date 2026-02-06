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

	// 배경 위젯 생성 (항상 밑에 유지)
	if (BackgroundWidgetClass)
	{
		BackgroundWidget = CreateWidget<UUserWidget>(this, BackgroundWidgetClass);
		if (BackgroundWidget)
		{
			BackgroundWidget->AddToViewport(-100);
		}
	}

	// 처음에 로비 표시
	ShowLobby();
}

void ABrawlLobbyPlayerController::ShowLobby()
{
	// 기존 위젯 제거
	if (CurrentMainWidget)
	{
		CurrentMainWidget->RemoveFromParent();
		CurrentMainWidget = nullptr;
	}

	if (LobbyWidgetClass)
	{
		CurrentMainWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
		if (CurrentMainWidget)
		{
			CurrentMainWidget->AddToViewport();
		}
	}
}

void ABrawlLobbyPlayerController::ShowBrawlerSelect()
{
	// 기존 위젯 제거
	if (CurrentMainWidget)
	{
		CurrentMainWidget->RemoveFromParent();
		CurrentMainWidget = nullptr;
	}

	if (BrawlerSelectWidgetClass)
	{
		CurrentMainWidget = CreateWidget<UUserWidget>(this, BrawlerSelectWidgetClass);
		if (CurrentMainWidget)
		{
			CurrentMainWidget->AddToViewport();
		}
	}
}
