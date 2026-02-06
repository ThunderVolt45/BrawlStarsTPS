// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlBrawlerSelectWidget.generated.h"

class UButton;
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

	/** 나가기 버튼 클릭 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void OnExitClicked();

	/** 브롤러가 최종적으로 선택되었을 때(버튼 클릭 등) 발생하는 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|UI")
	void OnBrawlerSelected();

protected:
	virtual void NativeConstruct() override;
	
	
	UPROPERTY(meta=(BindWidget))
	UButton* ButtonExit;
};
