// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlMatchResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "TimerManager.h"

void UBrawlMatchResultWidget::SetupResult(bool bInIsWinner, int32 InRank)
{
	bIsWinner = bInIsWinner;
	Rank = InRank;

	// C++ 로직 및 블루프린트 이벤트 호출
	OnResultApplied();
}

void UBrawlMatchResultWidget::HandleExitClicked()
{
	OnExitClicked.Broadcast();
}

void UBrawlMatchResultWidget::OnResultApplied_Implementation()
{
	if (TextBlock)
	{
		FText ResultText = bIsWinner ? FText::FromString(TEXT("승리")) : FText::FromString(TEXT("패배"));
		TextBlock->SetText(ResultText);
	}

	if (ExitButton)
	{
		// 처음에는 나가기 버튼을 숨김 (연출을 위해)
		ExitButton->SetVisibility(ESlateVisibility::Hidden);
		
		// 나가기 버튼에 이벤트 부착
		ExitButton->OnClicked.AddDynamic(this, &UBrawlMatchResultWidget::HandleExitClicked);

		// 일정 시간 뒤 버튼 활성화 (타이머 설정)
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, 
			&UBrawlMatchResultWidget::DelayedEnableExitButton, ExitButtonDelay, false);
	}
}

void UBrawlMatchResultWidget::DelayedEnableExitButton()
{
	if (ExitButton)
	{
		ExitButton->SetVisibility(ESlateVisibility::Visible);
	}
}

void UBrawlMatchResultWidget::ShowFinalResult_Implementation()
{
	if (TextBlock)
	{
		// 최종 결과 텍스트로 변경 (예: "RANK 1")
		FText FinalText = FText::Format(FText::FromString(TEXT("RANK {0}")), FText::AsNumber(Rank));
		TextBlock->SetText(FinalText);
	}

	if (ExitButton)
	{
		// 최종 결과창에서는 바로 버튼을 보여줌
		ExitButton->SetVisibility(ESlateVisibility::Visible);
	}
}