// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlGameState_Bounty.h"
#include "Net/UnrealNetwork.h"

ABrawlGameState_Bounty::ABrawlGameState_Bounty()
{
	Team0Score = 0;
	Team1Score = 0;
	RemainingTime = 120; // 기본 2분
}

void ABrawlGameState_Bounty::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABrawlGameState_Bounty, Team0Score);
	DOREPLIFETIME(ABrawlGameState_Bounty, Team1Score);
	DOREPLIFETIME(ABrawlGameState_Bounty, RemainingTime);
}

void ABrawlGameState_Bounty::AddTeamScore(int32 TeamID, int32 Amount)
{
	if (HasAuthority())
	{
		if (TeamID == 0) Team0Score += Amount;
		else if (TeamID == 1) Team1Score += Amount;
	}
}

int32 ABrawlGameState_Bounty::GetTeamScore(int32 TeamID) const
{
	if (TeamID == 0) return Team0Score;
	if (TeamID == 1) return Team1Score;
	return 0;
}

void ABrawlGameState_Bounty::SetRemainingTime(int32 TimeInSeconds)
{
	if (HasAuthority())
	{
		RemainingTime = TimeInSeconds;
	}
}
