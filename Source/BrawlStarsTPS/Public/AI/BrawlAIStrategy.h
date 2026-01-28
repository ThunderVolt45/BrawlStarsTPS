// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlAIStrategy.generated.h"

/**
 * AI의 전략 상태를 정의하는 Enum
 */
UENUM(BlueprintType)
enum class EBrawlAIStrategy : uint8
{
	Patrol = 0	UMETA(DisplayName = "Patrol"),	// 순찰
	Move = 1	UMETA(DisplayName = "Move"),	// 추적/이동
	Combat = 2	UMETA(DisplayName = "Combat"),	// 교전
	Flee = 3	UMETA(DisplayName = "Flee")		// 도주
};
