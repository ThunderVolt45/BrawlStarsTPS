#include "BrawlGameState_Knockout.h"
#include "Net/UnrealNetwork.h"

ABrawlGameState_Knockout::ABrawlGameState_Knockout()
{
}

void ABrawlGameState_Knockout::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABrawlGameState_Knockout, Team0Wins);
	DOREPLIFETIME(ABrawlGameState_Knockout, Team1Wins);
	DOREPLIFETIME(ABrawlGameState_Knockout, LastRoundWinner);
	DOREPLIFETIME(ABrawlGameState_Knockout, RoundWinners);
}

void ABrawlGameState_Knockout::SetTeamWins(int32 InTeam0Wins, int32 InTeam1Wins)
{
	if (HasAuthority())
	{
		Team0Wins = InTeam0Wins;
		Team1Wins = InTeam1Wins;
	}
}

void ABrawlGameState_Knockout::SetLastRoundWinner(int32 Winner)
{
	if (HasAuthority())
	{
		LastRoundWinner = Winner;
	}
}

void ABrawlGameState_Knockout::AddRoundWinner(int32 Winner)
{
	if (HasAuthority())
	{
		RoundWinners.Add(Winner);
	}
}
