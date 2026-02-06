// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "Data/BrawlTypes.h"
#include "BrawlModeSelectWidget.generated.h"

/**
 * UBrawlModeSelectWidget
 * 
 * 게임 모드 선택 화면 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlModeSelectWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	/** 게임 모드 선택 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SelectMode(EBrawlGameModeType ModeType, FName MapName);

protected:
	/** 모드 리스트를 가져올 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Data")
	TObjectPtr<UDataTable> ModeDataTable;
};
