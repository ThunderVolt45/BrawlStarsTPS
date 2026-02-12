// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "BrawlStarsTPSGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBrawlerKilled, AActor*, Killer, AActor*, Victim);

/**
 * ABrawlStarsTPSGameMode
 * BrawlStarsTPS 프로젝트의 공통 게임 모드 베이스 클래스
 */
UCLASS(abstract)
class ABrawlStarsTPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Constructor */
	ABrawlStarsTPSGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 처치 발생 시 호출
	virtual void NotifyKill(AActor* Killer, AActor* Victim);

	// 이 게임 모드에서 해당 캐릭터가 리스폰되어야 하는지 여부 반환
	virtual bool ShouldRespawn(AActor* Victim) const { return false; }

	// 컨트롤러에 따른 기본 폰 클래스 반환 오버라이드
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	// 플레이어 시작 지점 선택
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** 게임 시작 실행 (Intro 연출 종료 후 호출) */
	UFUNCTION(BlueprintCallable, Category = "Brawl|GameMode")
	virtual void StartMatch();

	/** 게임 종료 처리 */
	virtual void EndGame(bool bIsPlayerWinner, int32 RankOrTeam);

protected:
	/** 브롤러 ID와 클래스 매핑을 위한 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Data")
	TObjectPtr<UDataTable> BrawlerClassDataTable;

	// --- Match Flow ---
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|GameMode")
	float StartDelay = 5.0f;

	bool bHasMatchStarted = false;

	// --- AI & Spawning ---
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|GameMode")
	TArray<TSubclassOf<class ABrawlCharacter>> AICharacterClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|GameMode")
	int32 MaxBots = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|AI")
	TObjectPtr<class UBehaviorTree> GameModeAITree;

	virtual void SpawnBots();
	virtual void ConfigureAI(class AController* AIController, int32 TeamID);

	// --- Poison Zone Logic (Common to Showdown/Knockout) ---
	void StartPoisonLogic();
	virtual void UpdatePoisonZone();
	virtual void CheckPoisonDamage();

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Poison")
	TSubclassOf<class ABrawlPoisonZone> PoisonZoneClass;

	UPROPERTY()
	TObjectPtr<class ABrawlPoisonZone> PoisonZoneInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Poison")
	TSubclassOf<class UGameplayEffect> PoisonDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Poison")
	float InitialSafeZoneRadius = 4000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Poison")
	float MinSafeZoneRadius = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Poison")
	float PoisonShrinkSpeed = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Poison")
	float PoisonDamage = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Poison")
	float PoisonDamageInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Poison")
	float PoisonStartDelay = 10.0f;

	float CurrentSafeZoneRadius;
	FTimerHandle PoisonUpdateTimerHandle;
	FTimerHandle PoisonDamageTimerHandle;
};



