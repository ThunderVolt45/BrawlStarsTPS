#pragma once

#include "CoreMinimal.h"
#include "BrawlGameState.h"
#include "BrawlGameState_Knockout.generated.h"

/**
 * ABrawlGameState_Knockout
 * 
 * 녹아웃 모드 전용 게임 상태 클래스
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlGameState_Knockout : public ABrawlGameState
{
	GENERATED_BODY()

public:
	ABrawlGameState_Knockout();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetTeamWins(int32 InTeam0Wins, int32 InTeam1Wins);
	void SetLastRoundWinner(int32 Winner);

	UFUNCTION(BlueprintCallable, Category = "Brawl|Knockout")
	int32 GetTeam0Wins() const { return Team0Wins; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Knockout")
	int32 GetTeam1Wins() const { return Team1Wins; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Knockout")
	int32 GetLastRoundWinner() const { return LastRoundWinner; }

protected:
	// 팀 0 (레드) 승리 횟수
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|Knockout")
	int32 Team0Wins = 0;

	// 팀 1 (블루) 승리 횟수
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|Knockout")
	int32 Team1Wins = 0;

	// 마지막 라운드 승리 팀 (-1: 무승부, 0: 레드, 1: 블루)
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|Knockout")
	int32 LastRoundWinner = -1;
};
