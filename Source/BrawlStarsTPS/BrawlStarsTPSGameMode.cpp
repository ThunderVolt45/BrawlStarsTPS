// Copyright Epic Games, Inc. All Rights Reserved.

#include "BrawlStarsTPSGameMode.h"
#include "UObject/ConstructorHelpers.h"

ABrawlStarsTPSGameMode::ABrawlStarsTPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ABrawlStarsTPSGameMode::NotifyKill(AActor* Killer, AActor* Victim)
{
	if (OnBrawlerKilled.IsBound())
	{
		OnBrawlerKilled.Broadcast(Killer, Victim);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Kill Confirmed: [%s] killed [%s]"), 
		Killer ? *Killer->GetName() : TEXT("Environment"), 
		Victim ? *Victim->GetName() : TEXT("Unknown"));
}
