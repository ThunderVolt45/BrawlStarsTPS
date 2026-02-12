// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlGameState.h"
#include "Net/UnrealNetwork.h"
#include "AI/BrawlAIController.h"
#include "Kismet/GameplayStatics.h"

void ABrawlGameState::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 AI 컨트롤러들을 미리 캐싱해둠
	if (HasAuthority())
	{
		TArray<AActor*> FoundAI;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlAIController::StaticClass(), FoundAI);
		for (AActor* Actor : FoundAI)
		{
			if (ABrawlAIController* AIC = Cast<ABrawlAIController>(Actor))
			{
				CachedAIControllers.Add(AIC);
				// 시작 시에는 AI를 꺼둠
				AIC->SetAIActive(false);
			}
		}
	}
}

void ABrawlGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABrawlGameState, AliveBrawlerCount);
	DOREPLIFETIME(ABrawlGameState, MatchState);
	DOREPLIFETIME(ABrawlGameState, ModeName);
	DOREPLIFETIME(ABrawlGameState, ModeDescription);
}

void ABrawlGameState::SetMatchState(EBrawlMatchState NewState)
{
	if (HasAuthority() && MatchState != NewState)
	{
		MatchState = NewState;
		
		// AI 활성화 제어 (Playing 상태일 때만 켬)
		SetAllAIActive(MatchState == EBrawlMatchState::Playing);

		// 서버(리슨 서버 포함)에서도 로컬 연출을 위해 브로드캐스트 수행
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_MatchState();
		}
	}
}

void ABrawlGameState::OnRep_MatchState()
{
	// 클라이언트에서 연출 트리거를 위해 브로드캐스트
	OnMatchStateChanged.Broadcast();
}

void ABrawlGameState::SetModeInfo(FText InName, FText InDescription)
{
	if (HasAuthority())
	{
		ModeName = InName;
		ModeDescription = InDescription;
	}
}

void ABrawlGameState::SetAllAIActive(bool bActive)
{
	if (HasAuthority())
	{
		// 실시간으로 월드의 모든 AI 컨트롤러 검색 (동적 생성/리스폰 대응)
		TArray<AActor*> FoundAI;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlAIController::StaticClass(), FoundAI);

		for (AActor* Actor : FoundAI)
		{
			if (ABrawlAIController* AIC = Cast<ABrawlAIController>(Actor))
			{
				AIC->SetAIActive(bActive);
			}
		}
	}
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
