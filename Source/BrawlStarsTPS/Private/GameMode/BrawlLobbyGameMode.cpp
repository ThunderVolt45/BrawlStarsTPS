// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BrawlLobbyGameMode.h"
#include "UI/BrawlLobbyPlayerController.h"

ABrawlLobbyGameMode::ABrawlLobbyGameMode()
{
	PlayerControllerClass = ABrawlLobbyPlayerController::StaticClass();
	DefaultPawnClass = nullptr; // 로비에서는 폰이 필요 없을 수 있음 (카메라 액터만 배치)
}
