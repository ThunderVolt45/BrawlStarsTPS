// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlMatchResultWidget.generated.h"

/**
 * UBrawlMatchResultWidget
 * 
 * 게임 종료 시 승리/패배 및 순위를 표시하는 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlMatchResultWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	/** 결과 데이터 설정 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetupResult(bool bInIsWinner, int32 InRank);

protected:
	/** 블루프린트에서 승패에 따른 연출(VFX, 애니메이션 등)을 처리하기 위한 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|UI")
	void OnResultApplied();

	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	bool bIsWinner;

	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	int32 Rank;
};
