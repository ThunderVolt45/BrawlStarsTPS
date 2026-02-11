// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlBountyWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * UBrawlBountyWidget
 * 
 * 바운티 모드 전용 UI 위젯
 * - 좌측: 아군 팀 점수
 * - 우측: 적군 팀 점수
 * - 중앙: 남은 시간
 * - 타이 브레이커 아이콘 상태 표시
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlBountyWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/** 팀 ID를 찾을 때까지 주기적으로 실행될 함수 */
	void CheckTeamID();

	/** 팀 점수 업데이트 콜백 */
	UFUNCTION()
	void OnTeamScoreChanged(int32 TeamID, int32 NewScore);

	/** 남은 시간 업데이트 콜백 */
	UFUNCTION()
	void OnRemainingTimeChanged(int32 NewTime);

	/** 타이 브레이커 변경 콜백 */
	UFUNCTION()
	void HandleTieBreakerChanged(int32 TeamID);

	/** UI 텍스트 및 아이콘 상태 갱신 */
	void UpdateBountyUI();

protected:
	// --- UI Components (BindWidget) ---
	
	// 좌측 (아군) 팀 점수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LeftTeamScoreText;

	// 우측 (적군) 팀 점수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RightTeamScoreText;

	// 경기 남은 시간
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;

	// 좌측 팀 타이 브레이커 아이콘
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LeftTieBreakerIcon;

	// 우측 팀 타이 브레이커 아이콘
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RightTieBreakerIcon;

private:
	/** 현재 로컬 플레이어의 팀 ID 캐싱 */
	int32 MyTeamID = 255;

	FTimerHandle TeamCheckTimerHandle;
};
