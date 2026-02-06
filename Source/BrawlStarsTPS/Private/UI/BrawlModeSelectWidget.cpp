// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlModeSelectWidget.h"
#include "BrawlGameInstance.h"

void UBrawlModeSelectWidget::SelectMode(EBrawlGameModeType ModeType, FName MapName)
{
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->SelectedGameModeType = ModeType;
		GI->SelectedMapName = MapName;
		
		// 선택 후 로비로 돌아가거나 하는 처리
	}
}
