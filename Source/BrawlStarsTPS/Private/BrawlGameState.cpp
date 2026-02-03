// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlGameState.h"
#include "Net/UnrealNetwork.h"

void ABrawlGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ABrawlGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABrawlGameState, AliveBrawlerCount);
}

void ABrawlGameState::SetAliveBrawlerCount(int32 Count)
{
	if (HasAuthority())
	{
		AliveBrawlerCount = Count;
	}
}

void ABrawlGameState::NotifyBrawlerKilled(AActor* Killer, AActor* Victim)
{
	// 서버에서만 실행되도록 보장
	if (HasAuthority())
	{
		MulticastOnBrawlerKilled(Killer, Victim);
	}
}

void ABrawlGameState::MulticastOnBrawlerKilled_Implementation(AActor* Killer, AActor* Victim)
{
	UE_LOG(LogTemp, Log, TEXT("BrawlGameState: MulticastOnBrawlerKilled. Killer: %s, Victim: %s"), 
		Killer ? *Killer->GetName() : TEXT("None"), 
		Victim ? *Victim->GetName() : TEXT("None"));

	// 모든 클라이언트(및 리슨 서버)의 UI나 이펙트 시스템에 알림
	if (OnBrawlerKilled.IsBound())
	{
		OnBrawlerKilled.Broadcast(Killer, Victim);
	}
}
