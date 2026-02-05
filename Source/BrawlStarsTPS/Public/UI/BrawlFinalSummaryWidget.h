// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlFinalSummaryWidget.generated.h"

/**
 * UBrawlFinalSummaryWidget
 * 
 * 레벨 정리 후 나타나는 최종 결과(순위, 보상 등) 화면 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlFinalSummaryWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	/** 최종 데이터 설정 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetupFinalSummary(int32 InRank);

protected:
	/** 블루프린트에서 데이터 세팅 후 연출을 위한 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|UI")
	void OnSummaryApplied();

	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	int32 Rank;
};
