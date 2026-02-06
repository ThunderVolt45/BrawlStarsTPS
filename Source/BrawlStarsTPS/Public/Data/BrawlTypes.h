// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlTypes.generated.h"

/**
 * EBrawlGameModeType
 * 
 * 게임 모드의 타입을 구분합니다.
 */
UENUM(BlueprintType)
enum class EBrawlGameModeType : uint8
{
	None UMETA(DisplayName = "None"),
	Showdown UMETA(DisplayName = "Showdown"),
	GemGrab UMETA(DisplayName = "Gem Grab"),
	BrawlBall UMETA(DisplayName = "Brawl Ball"),
	Heist UMETA(DisplayName = "Heist"),
	Bounty UMETA(DisplayName = "Bounty")
};
