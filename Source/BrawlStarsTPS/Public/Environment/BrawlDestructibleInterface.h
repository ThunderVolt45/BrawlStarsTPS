// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BrawlDestructibleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBrawlDestructibleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IBrawlDestructibleInterface
 * 
 * 파괴 가능한 환경 오브젝트(벽, 상자 등)가 구현해야 할 인터페이스
 * BrawlProjectile 등에서 이 인터페이스를 캐스팅하여 파괴 로직을 수행함
 */
class BRAWLSTARSTPS_API IBrawlDestructibleInterface
{
	GENERATED_BODY()

public:
	// 파괴 가능한 상태인지 확인 (예: 이미 파괴되었거나, 특정 조건에서만 파괴 가능)
	virtual bool IsDestructible() const = 0;
	
	// "단단한" 장애물인지 확인 (예: 벽 = 단단함, 수풀 != 단단함)
	virtual bool IsHardObstacle() const = 0;

	// 파괴 요청이 들어왔을 때 호출 (Instigator: 파괴를 유발한 행위자)
	virtual void OnDestruction(AActor* InstigatorActor) = 0;
};
