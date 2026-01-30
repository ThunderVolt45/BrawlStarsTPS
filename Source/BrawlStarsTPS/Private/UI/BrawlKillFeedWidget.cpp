// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlKillFeedWidget.h"
#include "UI/BrawlKillLogEntry.h"
#include "BrawlStarsTPSGameMode.h" // GameMode는 이제 필요 없을 수 있지만, 혹시 모르니 유지하거나 제거
#include "BrawlGameState.h"      // 추가
#include "BrawlCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PanelWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"

void UBrawlKillFeedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// GameState를 통해 킬 이벤트 수신 (클라이언트에서도 접근 가능)
	if (ABrawlGameState* GS = Cast<ABrawlGameState>(UGameplayStatics::GetGameState(GetWorld())))
	{
		GS->OnBrawlerKilled.AddDynamic(this, &UBrawlKillFeedWidget::HandleBrawlerKilled);
		UE_LOG(LogTemp, Log, TEXT("BrawlKillFeedWidget: Successfully bound to BrawlGameState OnBrawlerKilled."));
	}
	else
	{
		// GameState가 아직 복제되지 않았을 수도 있음 (비동기). 
		// 하지만 NativeConstruct는 보통 GameState가 있을 때 쯤 호출됨.
		// 만약 실패한다면, Timer를 써서 재시도하거나 OnGameStateSet 이벤트를 기다려야 함.
		// 여기서는 간단히 로그 출력.
		UE_LOG(LogTemp, Error, TEXT("BrawlKillFeedWidget: Failed to cast GameState to ABrawlGameState! Current: %s"), 
			UGameplayStatics::GetGameState(GetWorld()) ? *UGameplayStatics::GetGameState(GetWorld())->GetName() : TEXT("NULL"));
	}

	if (!KillFeedList)
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlKillFeedWidget: KillFeedList is NULL! Check Widget Name in Blueprint."));
	}
	if (!KillLogEntryClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlKillFeedWidget: KillLogEntryClass is NULL! Set it in Blueprint Details."));
	}
}

void UBrawlKillFeedWidget::HandleBrawlerKilled(AActor* Killer, AActor* Victim)
{
	UE_LOG(LogTemp, Log, TEXT("BrawlKillFeedWidget: HandleBrawlerKilled Called. Killer: %s, Victim: %s"), 
		Killer ? *Killer->GetName() : TEXT("None"), 
		Victim ? *Victim->GetName() : TEXT("None"));

	if (!KillLogEntryClass || !KillFeedList)
	{
		return;
	}

	// 1. 위젯 생성
	UBrawlKillLogEntry* NewEntry = CreateWidget<UBrawlKillLogEntry>(this, KillLogEntryClass);
	if (!NewEntry)
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlKillFeedWidget: Fail to Create KillLogWidget!"));
		return;
	}

	// 2. 정보 설정 (액터 자체를 넘겨서 Entry 위젯이 처리하도록 함)
	NewEntry->SetKillInfo(Killer, Victim);

	// 3. 리스트에 추가
	KillFeedList->AddChild(NewEntry);

	// 4. 최대 개수 유지 (오래된 것 삭제)
	if (KillFeedList->GetChildrenCount() > MaxLogEntries)
	{
		// 0번 인덱스(가장 오래된 것) 삭제
		if (UWidget* OldestChild = KillFeedList->GetChildAt(0))
		{
			OldestChild->RemoveFromParent();
		}
	}
}
