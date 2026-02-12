#include "UI/BrawlRoundResultWidget.h"
#include "Components/TextBlock.h"

void UBrawlRoundResultWidget::SetupRoundResult(bool bInIsWinner, int32 InTeam1Score, int32 InTeam2Score)
{
	bIsWinner = bInIsWinner;
	Team1Score = InTeam1Score;
	Team2Score = InTeam2Score;

	if (ResultText)
	{
		ResultText->SetText(FText::FromString(bIsWinner ? TEXT("라운드 승리") : TEXT("라운드 패배")));
	}

	OnRoundResultApplied();
}

void UBrawlRoundResultWidget::OnRoundResultApplied_Implementation()
{
	// 기본 구현은 비워둠 (BP에서 오버라이드)
}
