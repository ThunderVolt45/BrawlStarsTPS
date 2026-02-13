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
	Bounty UMETA(DisplayName = "Bounty"),
	KnockOut UMETA(DisplayName = "Knock Out")
};

/**
 * EBrawlCharacterType
 * 캐릭터의 근본적인 역할을 구분합니다.
 */
UENUM(BlueprintType)
enum class EBrawlCharacterType : uint8
{
	Hero    UMETA(DisplayName = "Hero"),    // 플레이어/봇이 선택하는 주역 브롤러
	Summon  UMETA(DisplayName = "Summon"),  // 가젯, 궁극기 등으로 생성된 소환물
	Etc     UMETA(DisplayName = "Etc")     // 상자 등 기타 파괴 가능한 오브젝트
};
