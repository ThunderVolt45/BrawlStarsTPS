// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlBrawlerSelectWidget.h"
#include "BrawlGameInstance.h"

void UBrawlBrawlerSelectWidget::SelectBrawler(FName BrawlerRowName)
{
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->SelectedBrawlerRowName = BrawlerRowName;
		
		// 선택 후 로비로 돌아가거나 하는 처리 (Blueprint에서 UI 애니메이션과 함께 처리 권장)
	}
}
