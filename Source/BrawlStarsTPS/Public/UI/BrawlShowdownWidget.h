// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BrawlShowdownWidget.generated.h"

class UTextBlock;
class ABrawlGameState;

/**
 * UBrawlShowdownWidget
 * 
 * 쇼다운 모드 전용 UI 위젯입니다.
 * 남은 생존자 수를 표시합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlShowdownWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// 생존자 수 업데이트 함수
	void UpdateAliveCount();

public:
	// 생존자 수 텍스트 (BindWidget)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AliveCountText;

	// '남은 사람' 라벨 등 (Optional)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LabelText;

protected:
	// 캐싱된 게임 스테이트
	TWeakObjectPtr<ABrawlGameState> BrawlGameState;

	// 마지막으로 표시한 생존자 수 (불필요한 텍스트 갱신 방지)
	int32 LastAliveCount = -1;
};
