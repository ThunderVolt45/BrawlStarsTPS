// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "Data/BrawlTypes.h"
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

	/** 플레이어 입장 시 호출 (팀 할당 로직 포함) */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** 플레이어 스폰 전 초기화 (RestartPlayer 호출 전) */
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	/** 브롤러 ID와 클래스 매핑을 위한 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Data")
	TObjectPtr<UDataTable> BrawlerClassDataTable;

	/** 게임 모드 정보를 담은 스트링 테이블 에셋 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Data")
	TSoftObjectPtr<class UStringTable> GameModeStringTable;

	/** 이 게임 모드의 식별자 (스트링 테이블 키의 접두사로 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Data")
	FName GameModeID = FName("Showdown");

	/** 이 게임 모드의 타입 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Data")
	EBrawlGameModeType GameModeType = EBrawlGameModeType::None;

	// --- Match Flow ---
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|GameMode")
	float StartDelay = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, Category= "Brawl|GameMode")
	float RoundDelay = 3.0f;

	bool bHasMatchStarted = false;

protected:
	// --- AI & Spawning (공통 로직으로 통합) ---
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|GameMode")
	TArray<TSubclassOf<class ABrawlCharacter>> AICharacterClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|GameMode")
	int32 MaxBots = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|AI")
	TObjectPtr<class UBehaviorTree> GameModeAITree;

	/** 컨트롤러별 팀 관리 (Bounty/Knockout 공통 사용) */
	UPROPERTY()
	TMap<TObjectPtr<AController>, int32> AssignedTeams;

	/** 컨트롤러의 팀 ID를 안전하게 조회 (맵 또는 게임 모드 규칙 기반) */
	virtual int32 GetControllerTeamID(AController* InController) const;

	/** AI 컨트롤러별 캐릭터 클래스 관리 */
	UPROPERTY()
	TMap<TObjectPtr<AController>, TSubclassOf<class ABrawlCharacter>> AssignedAIClasses;

	/** 초기 팀 설정 (플레이어 자동 배치 등) */
	virtual void SetupTeams();

	/** AI 봇 생성 공통 로직 */
	virtual void SpawnBots();

	/** 특정 팀과 위치에 봇 하나 스폰 (헬퍼) */
	virtual bool SpawnBotAt(int32 TeamID, class ABrawlSpawnPoint* SP);

	/** AI 컨트롤러 설정 (팀 ID 주입 등) */
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



