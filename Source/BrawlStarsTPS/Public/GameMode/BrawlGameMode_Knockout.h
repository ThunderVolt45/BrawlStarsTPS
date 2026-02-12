#pragma once

#include "CoreMinimal.h"
#include "BrawlStarsTPSGameMode.h"
#include "BrawlGameMode_Knockout.generated.h"

/**
 * ABrawlGameMode_Knockout
 * 녹아웃 모드 (3v3, 3판 2선승제)
 * - 브롤러 사망 시 해당 라운드 종료까지 리스폰 불가
 * - 상대 팀 전멸 시 라운드 승리
 * - 일정 시간 후 독구름 생성
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlGameMode_Knockout : public ABrawlStarsTPSGameMode
{
	GENERATED_BODY()

public:
	ABrawlGameMode_Knockout();

	virtual void BeginPlay() override;
	virtual void NotifyKill(AActor* Killer, AActor* Victim) override;
	virtual void StartMatch() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 녹아웃은 라운드 종료 후 일괄 리스폰하므로 캐릭터를 파괴하지 않음
	virtual bool ShouldRespawn(AActor* Victim) const override { return true; }

protected:
	// 새로운 라운드 시작
	void StartNewRound();

	// 라운드 종료 체크
	void CheckRoundEndCondition();

	// 라운드 결과 처리
	void EndRound(int32 WinningTeam);

	// 모든 브롤러 리스폰 및 초기 위치 이동
	void ResetBrawlersForRound();

	// 초기 팀 설정
	void SetupTeams();

	// AI 봇 생성 (3v3에 맞춰 5명 추가 생성)
	virtual void SpawnBots() override;

protected:
	UPROPERTY(VisibleInstanceOnly, Category = "Brawl|Knockout")
	int32 CurrentRound = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Brawl|Knockout")
	int32 Team1Wins = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Brawl|Knockout")
	int32 Team2Wins = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Brawl|Knockout")
	int32 Team1AliveCount = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Brawl|Knockout")
	int32 Team2AliveCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Knockout")
	int32 MaxRounds = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Knockout")
	int32 RequiredWins = 2;

	// 라운드 간 대기 시간
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Knockout")
	float RoundResetDelay = 3.0f;

	FTimerHandle RoundStartTimerHandle;
};