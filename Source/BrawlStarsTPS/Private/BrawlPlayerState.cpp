// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlPlayerState.h"
#include "Net/UnrealNetwork.h"

ABrawlPlayerState::ABrawlPlayerState()
{
	CurrentBounty = 2;
	ScoreContribution = 0;
	bHasTieBreaker = false;
}

void ABrawlPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABrawlPlayerState, CurrentBounty);
	DOREPLIFETIME(ABrawlPlayerState, ScoreContribution);
	DOREPLIFETIME(ABrawlPlayerState, TeamID);
	DOREPLIFETIME(ABrawlPlayerState, bHasTieBreaker);
}

void ABrawlPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (HasAuthority())
	{
		TeamID = NewTeamID;
	}
}

FGenericTeamId ABrawlPlayerState::GetGenericTeamId() const
{
	return TeamID;
}

void ABrawlPlayerState::SetBounty(int32 NewBounty)
{
	if (HasAuthority())
	{
		CurrentBounty = FMath::Clamp(NewBounty, 2, 7);
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_CurrentBounty();
		}
	}
}

void ABrawlPlayerState::AddBounty(int32 Amount)
{
	SetBounty(CurrentBounty + Amount);
}

void ABrawlPlayerState::ResetBounty()
{
	SetBounty(2);
}

void ABrawlPlayerState::AddScoreContribution(int32 Amount)
{
	if (HasAuthority())
	{
		ScoreContribution += Amount;
	}
}

void ABrawlPlayerState::SetTeamID(int32 NewTeamID)
{
	SetGenericTeamId(FGenericTeamId(NewTeamID));
}

void ABrawlPlayerState::SetHasTieBreaker(bool bHas)
{
	if (HasAuthority())
	{
		bHasTieBreaker = bHas;
	}
}

void ABrawlPlayerState::OnRep_CurrentBounty()
{
	OnBountyChanged.Broadcast(CurrentBounty);
}

void ABrawlPlayerState::OnRep_TeamID()
{
	// 필요 시 팀 ID 변경에 따른 처리 (UI 등)
}
