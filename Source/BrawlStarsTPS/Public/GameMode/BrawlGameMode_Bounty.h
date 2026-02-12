// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlStarsTPSGameMode.h"
#include "BrawlGameMode_Bounty.generated.h"

class ABrawlCharacter;
class ABrawlTieBreaker;

/**
 * ABrawlGameMode_Bounty
 * 
 * 바운티 게임 모드
 * - 3v3 팀전
 * - 처치 시 점수 획득 및 현상금 증가
 * - 20점 도달 또는 시간 종료 시 승리
 * - 동점 시 타이 브레이커 소유 팀 승리
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlGameMode_Bounty : public ABrawlStarsTPSGameMode
{
	GENERATED_BODY()

public:
	ABrawlGameMode_Bounty();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 플레이어 입장 시 호출
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 플레이어 시작 지점 선택 (부모 클래스 오버라이드)
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 컨트롤러별 생성할 폰 클래스 반환
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	// 처치 발생 시 호출
	virtual void NotifyKill(AActor* Killer, AActor* Victim) override;

	// 바운티 모드에서는 리스폰 필요
	virtual bool ShouldRespawn(AActor* Victim) const override { return true; }

	// 게임 시작 실행
	virtual void StartMatch() override;

	// 타이 브레이커 아이템 획득 시 호출
	void OnTieBreakerPickedUp(ABrawlCharacter* Picker);

protected:
	// 팀 및 봇 설정
	void SetupTeams();
	virtual void SpawnBots() override;

	// 리스폰 처리
	void RequestRespawn(AController* Controller);

	UFUNCTION()
	void RespawnBrawler(AController* Controller);

	// 게임 종료 체크
	void CheckWinCondition();
	virtual void EndGame(bool bIsPlayerWinner, int32 RankOrTeam) override;

	// 매치 타이머 업데이트
	void UpdateMatchTimer();

	// 타이 브레이커 스폰
	void SpawnTieBreaker();

protected:
	// 경기 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Bounty")
	int32 MatchDuration = 120;

	// 목표 점수 (20점)
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Bounty")
	int32 TargetScore = 20;

	// 리스폰 대기 시간
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Bounty")
	float RespawnDelay = 3.0f;

	// 타이 브레이커 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Bounty")
	TSubclassOf<AActor> TieBreakerClass;

private:
	FTimerHandle MatchTimerHandle;
	bool bHasMatchStarted = false;
	
	// AI 컨트롤러와 할당된 캐릭터 클래스 매핑 (리스폰용)
	UPROPERTY()
	TMap<TObjectPtr<AController>, TSubclassOf<ABrawlCharacter>> AssignedAIClasses;

	// 컨트롤러별 팀 ID 직접 매핑 (PlayerState가 준비되기 전 스폰 단계에서 사용)
	UPROPERTY()
	TMap<TObjectPtr<AController>, int32> AssignedTeams;

	// 타이 브레이커 현재 소유자 (PlayerState)
	UPROPERTY()
	TObjectPtr<class ABrawlPlayerState> TieBreakerOwnerState;
};
