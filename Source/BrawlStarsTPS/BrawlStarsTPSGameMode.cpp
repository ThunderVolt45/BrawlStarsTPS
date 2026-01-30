// Copyright Epic Games, Inc. All Rights Reserved.

#include "BrawlStarsTPSGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "BrawlGameState.h"

ABrawlStarsTPSGameMode::ABrawlStarsTPSGameMode()
{
	// GameState 클래스 설정
	GameStateClass = ABrawlGameState::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ABrawlStarsTPSGameMode::NotifyKill(AActor* Killer, AActor* Victim)
{
	// 1. 서버 전용 로직 (점수 계산 등) - 필요 시 추가

	// 2. 모든 클라이언트에게 알림
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		GS->NotifyBrawlerKilled(Killer, Victim);
		UE_LOG(LogGameMode, Log, TEXT("GameMode: Called NotifyBrawlerKilled on GameState."));
	}
	else
	{
		UE_LOG(LogGameMode, Error, TEXT("GameMode: Failed to cast GameState to ABrawlGameState!"));
	}
	
	UE_LOG(LogGameMode, Log, TEXT("Kill Confirmed: [%s] killed [%s]"), 
		Killer ? *Killer->GetName() : TEXT("Environment"), 
		Victim ? *Victim->GetName() : TEXT("Unknown"));
}
