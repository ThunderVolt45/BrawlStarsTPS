// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlFinalSummaryWidget.h"

void UBrawlFinalSummaryWidget::SetupFinalSummary(int32 InRank)
{
	Rank = InRank;

	// 블루프린트 연출 시작
	OnSummaryApplied();
}
