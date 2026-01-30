// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BrawlStarsTPSGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBrawlerKilled, AActor*, Killer, AActor*, Victim);

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ABrawlStarsTPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	ABrawlStarsTPSGameMode();

	// 처치 발생 시 호출 (AttributeSet 등에서)
	virtual void NotifyKill(AActor* Killer, AActor* Victim);
};



