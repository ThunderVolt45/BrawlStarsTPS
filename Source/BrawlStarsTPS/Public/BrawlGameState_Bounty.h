// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlGameState.h"
#include "BrawlGameState_Bounty.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamScoreChanged, int32, TeamID, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChanged, int32, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTieBreakerTeamChanged, int32, TeamID);

/**
 * ABrawlGameState_Bounty
 * 
 * 바운티 모드 전용 게임 상태 클래스
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlGameState_Bounty : public ABrawlGameState
{
	GENERATED_BODY()

public:
	ABrawlGameState_Bounty();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 팀 점수 추가 */
	void AddTeamScore(int32 TeamID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Brawl|Bounty")
	int32 GetTeamScore(int32 TeamID) const;

	/** 남은 시간 설정 */
	void SetRemainingTime(int32 TimeInSeconds);

	UFUNCTION(BlueprintCallable, Category = "Brawl|Bounty")
	int32 GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Bounty")
	int32 GetTieBreakerTeam() const { return TieBreakerTeam; }

	/** 타이 브레이커 소유 팀 설정 */
	void SetTieBreakerTeam(int32 TeamID);

public:
	// UI 바인딩용 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Brawl|Bounty")
	FOnTeamScoreChanged OnTeamScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|Bounty")
	FOnRemainingTimeChanged OnRemainingTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Brawl|Bounty")
	FOnTieBreakerTeamChanged OnTieBreakerTeamChanged;

protected:
	UFUNCTION()
	void OnRep_Team0Score();

	UFUNCTION()
	void OnRep_Team1Score();

	UFUNCTION()
	void OnRep_RemainingTime();

	UFUNCTION()
	void OnRep_TieBreakerTeam();

protected:
	// 팀 0 점수
	UPROPERTY(ReplicatedUsing = OnRep_Team0Score, VisibleInstanceOnly, Category = "Brawl|Bounty")
	int32 Team0Score = 0;

	// 팀 1 점수
	UPROPERTY(ReplicatedUsing = OnRep_Team1Score, VisibleInstanceOnly, Category = "Brawl|Bounty")
	int32 Team1Score = 0;

	// 남은 경기 시간 (초)
	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, VisibleInstanceOnly, Category = "Brawl|Bounty")
	int32 RemainingTime = 0;

	// 타이 브레이커 소유 팀 ID (-1: 없음, 0: 레드, 1: 블루)
	UPROPERTY(ReplicatedUsing = OnRep_TieBreakerTeam, VisibleInstanceOnly, Category = "Brawl|Bounty")
	int32 TieBreakerTeam = -1;
};
