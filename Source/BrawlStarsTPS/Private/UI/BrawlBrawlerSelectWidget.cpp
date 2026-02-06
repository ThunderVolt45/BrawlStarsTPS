// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlBrawlerSelectWidget.h"
#include "BrawlGameInstance.h"

void UBrawlBrawlerSelectWidget::SelectBrawler(FName BrawlerRowName)
{
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->SelectedBrawlerRowName = BrawlerRowName;
		
		// 선택 완료 이벤트 호출 (BP에서 팝업 닫기 등 처리)
		OnBrawlerSelected();
	}
}
