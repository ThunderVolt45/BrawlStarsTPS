// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlBrawlerSelectWidget.generated.h"

/**
 * UBrawlBrawlerSelectWidget
 * 
 * 브롤러 선택 화면 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlBrawlerSelectWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	/** 브롤러 선택 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SelectBrawler(FName BrawlerRowName);

protected:
	/** 브롤러 리스트를 가져올 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Data")
	TObjectPtr<UDataTable> BrawlerDataTable;
};
