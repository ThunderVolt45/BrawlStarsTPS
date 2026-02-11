// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlGameState.h"
#include "BrawlGameState_Bounty.generated.h"

/**
 * ABrawlGameState_Bounty
 * 
 * 바운티 모드 전용 게임 상태 클래스
 * - 팀별 점수 관리
 * - 남은 시간 관리
 * - 승리 조건 도달 체크 (20점 선점)
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

protected:
	// 팀 0 점수
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|Bounty")
	int32 Team0Score = 0;

	// 팀 1 점수
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|Bounty")
	int32 Team1Score = 0;

	// 남은 경기 시간 (초)
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|Bounty")
	int32 RemainingTime = 0;
};
