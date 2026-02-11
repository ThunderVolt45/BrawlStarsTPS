// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlBountyWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "BrawlGameState_Bounty.h"
#include "BrawlPlayerState.h"
#include "Kismet/GameplayStatics.h"

void UBrawlBountyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// GameState 바인딩
	if (UWorld* World = GetWorld())
	{
		if (ABrawlGameState_Bounty* GS = World->GetGameState<ABrawlGameState_Bounty>())
		{
			GS->OnTeamScoreChanged.AddDynamic(this, &UBrawlBountyWidget::OnTeamScoreChanged);
			GS->OnRemainingTimeChanged.AddDynamic(this, &UBrawlBountyWidget::OnRemainingTimeChanged);

			// 초기 남은 시간 설정
			OnRemainingTimeChanged(GS->GetRemainingTime());
		}

		// 팀 ID 파악을 위해 0.2초마다 체크하는 타이머 시작
		World->GetTimerManager().SetTimer(TeamCheckTimerHandle, this, &UBrawlBountyWidget::CheckTeamID, 0.2f, true);
		// 즉시 1회 실행
		CheckTeamID();
	}
}

void UBrawlBountyWidget::CheckTeamID()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ABrawlPlayerState* PS = PC->GetPlayerState<ABrawlPlayerState>())
		{
			int32 CurrentID = PS->GetTeamID();
			// 팀 ID가 유효한 값(0 또는 1)으로 들어오면 처리 후 타이머 종료
			if (CurrentID != 255)
			{
				MyTeamID = CurrentID;
				UpdateBountyUI();

				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().ClearTimer(TeamCheckTimerHandle);
				}
				
				UE_LOG(LogTemp, Log, TEXT("BrawlBountyWidget: Successfully found TeamID %d. Stopping timer."), MyTeamID);
			}
		}
	}
}

void UBrawlBountyWidget::OnTeamScoreChanged(int32 TeamID, int32 NewScore)
{
	UpdateBountyUI();
}

void UBrawlBountyWidget::OnRemainingTimeChanged(int32 NewTime)
{
	if (TimerText)
	{
		int32 Minutes = NewTime / 60;
		int32 Seconds = NewTime % 60;
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, Seconds)));
	}
}

void UBrawlBountyWidget::UpdateBountyUI()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ABrawlGameState_Bounty* GS = World->GetGameState<ABrawlGameState_Bounty>();
	if (!GS) return;

	// 내 팀이 0이라면: 좌측(0), 우측(1)
	// 내 팀이 1이라면: 좌측(1), 우측(0)
	int32 EnemyTeamID = (MyTeamID == 0) ? 1 : 0;

	if (LeftTeamScoreText)
	{
		LeftTeamScoreText->SetText(FText::AsNumber(GS->GetTeamScore(MyTeamID)));
	}

	if (RightTeamScoreText)
	{
		RightTeamScoreText->SetText(FText::AsNumber(GS->GetTeamScore(EnemyTeamID)));
	}

	// 타이 브레이커 아이콘 색상 변경
	// (PlayerState를 순회하며 누가 타이 브레이커를 가졌는지 확인하거나 GS에 정보를 두는 것이 좋음)
	// 현재는 간단히 PlayerState들을 순회하여 체크
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerState::StaticClass(), OutActors);

	int32 TieBreakerTeam = -1;
	for (AActor* Actor : OutActors)
	{
		if (ABrawlPlayerState* PS = Cast<ABrawlPlayerState>(Actor))
		{
			if (PS->HasTieBreaker())
			{
				TieBreakerTeam = PS->GetTeamID();
				break;
			}
		}
	}

	if (LeftTieBreakerIcon)
	{
		LeftTieBreakerIcon->SetVisibility((TieBreakerTeam == MyTeamID) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (RightTieBreakerIcon)
	{
		RightTieBreakerIcon->SetVisibility((TieBreakerTeam == EnemyTeamID) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}
