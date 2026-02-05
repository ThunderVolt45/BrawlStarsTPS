// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlMatchResultWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchResultAction);

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

	/** 나가기 버튼 클릭 시 호출될 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Brawl|UI")
	FOnMatchResultAction OnExitClicked;

	/** 나가기 버튼 클릭 시 호출할 함수 (BP 버튼 이벤트에서 연결) */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void HandleExitClicked();

	/** 최종 결과 화면으로 전환 (레벨 정리 후 호출) */
	UFUNCTION(BlueprintNativeEvent, Category = "Brawl|UI")
	void ShowFinalResult();

	/** 블루프린트에서 승패에 따른 연출(VFX, 애니메이션 등)을 처리하기 위한 이벤트 */
	UFUNCTION(BlueprintNativeEvent, Category = "Brawl|UI")
	void OnResultApplied();

protected:
	/** 일정 시간 후 나가기 버튼 활성화 */
	void DelayedEnableExitButton();
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TextBlock;
	
	UPROPERTY(meta=(BindWidget))
	UButton* ExitButton;

	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	bool bIsWinner;

	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	int32 Rank;
	
	UPROPERTY(EditDefaultsOnly, Category="Brawl|UI")
	float ExitButtonDelay = 3.0f;
};
