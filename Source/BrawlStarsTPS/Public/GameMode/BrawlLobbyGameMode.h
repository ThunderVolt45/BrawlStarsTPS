// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BrawlLobbyGameMode.generated.h"

/**
 * ABrawlLobbyGameMode
 * 
 * 로비(아웃게임)를 위한 게임 모드입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABrawlLobbyGameMode();
};
