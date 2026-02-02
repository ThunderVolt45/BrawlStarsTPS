// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlSpawnPointType.generated.h"

/**
 * 스폰 포인트 타입
 */
UENUM(BlueprintType)
enum class EBrawlSpawnPointType : uint8
{
	Brawler UMETA(DisplayName = "Brawler"),
	PowerCubeBox UMETA(DisplayName = "Power Cube Box")
};
