// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BrawlAmmoSlotWidget.generated.h"

class UProgressBar;
class UImage;
class UWidgetAnimation;

/**
 * 개별 탄약 슬롯 위젯
 * - 충전 상태 표시 (ProgressBar)
 * - 완료 상태 표시 (Image/Color)
 * - 비어있음 표시
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlAmmoSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 초기화 (인덱스 저장 등)
	void InitSlot(int32 InSlotIndex);

	// 상태 업데이트
	// bIsFilled: 완전히 충전되었는지
	// bIsCharging: 현재 충전 중인지
	// ChargePercent: 충전 진행도 (0.0 ~ 1.0)
	void UpdateState(bool bIsFilled, bool bIsCharging, float ChargePercent);

	// 흔들림 애니메이션 재생
	void PlayShakeAnimation();

protected:
	virtual void NativeConstruct() override;

public:
	// 탄약 충전 게이지 (충전 중일 때만 진행도 표시)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> AmmoProgressBar;

	// 탄약이 꽉 찼을 때 보여줄 이미지 (선택 사항, ProgressBar Style로 대체 가능)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> FullAmmoImage;

	// 흔들림 애니메이션 (비어있을 때 발사 시도)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ShakeAnim;

protected:
	int32 SlotIndex = 0;
};
