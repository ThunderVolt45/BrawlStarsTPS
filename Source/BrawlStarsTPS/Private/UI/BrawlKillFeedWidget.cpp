// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlKillFeedWidget.h"
#include "UI/BrawlKillLogEntry.h"
#include "BrawlStarsTPSGameMode.h"
#include "BrawlCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PanelWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

void UBrawlKillFeedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ABrawlStarsTPSGameMode* GM = Cast<ABrawlStarsTPSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->OnBrawlerKilled.AddDynamic(this, &UBrawlKillFeedWidget::HandleBrawlerKilled);
		UE_LOG(LogTemp, Log, TEXT("BrawlKillFeedWidget: Successfully bound to BrawlStarsTPSGameMode OnBrawlerKilled."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlKillFeedWidget: Failed to cast GameMode to ABrawlStarsTPSGameMode! Current GameMode: %s"), 
			*UGameplayStatics::GetGameMode(GetWorld())->GetName());
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
