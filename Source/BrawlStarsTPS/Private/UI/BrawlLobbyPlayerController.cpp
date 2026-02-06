// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlLobbyPlayerController.h"
#include "UI/BrawlLobbyWidget.h"
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

	// 로비 위젯 생성 및 표시
	if (LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<UBrawlLobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
		}
	}
}
