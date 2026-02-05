// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlMatchResultWidget.h"

void UBrawlMatchResultWidget::SetupResult(bool bInIsWinner, int32 InRank)
{
	bIsWinner = bInIsWinner;
	Rank = InRank;

	// 블루프린트 이벤트 호출 (텍스트 변경, 사운드 재생 등)
	OnResultApplied();
}
