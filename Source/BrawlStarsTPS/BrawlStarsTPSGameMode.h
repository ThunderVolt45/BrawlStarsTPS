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

	// 컨트롤러에 따른 기본 폰 클래스 반환 오버라이드
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

protected:
	/** 브롤러 ID와 클래스 매핑을 위한 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Data")
	TObjectPtr<UDataTable> BrawlerClassDataTable;
};



