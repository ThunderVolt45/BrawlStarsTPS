// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlGameModeSelectWidget.generated.h"

class UButton;

/**
 * UBrawlGameModeSelectWidget
 * 
 * 게임 모드 선택 화면 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameModeSelectWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	/** 나가기 버튼 클릭 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void OnExitClicked();

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	UButton* ButtonExit;
};
