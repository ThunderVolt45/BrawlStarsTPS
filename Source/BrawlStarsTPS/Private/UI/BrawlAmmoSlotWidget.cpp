// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlAmmoSlotWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UBrawlAmmoSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBrawlAmmoSlotWidget::InitSlot(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
	UpdateState(false, false, 0.0f);
}

void UBrawlAmmoSlotWidget::UpdateState(bool bIsFilled, bool bIsCharging, float ChargePercent)
{
	if (AmmoProgressBar)
	{
		if (bIsFilled)
		{
			// 꽉 참
			AmmoProgressBar->SetPercent(1.0f);
			AmmoProgressBar->SetVisibility(ESlateVisibility::Visible);
			
			// 색상 변경 로직이 필요하다면 여기서 수행 (예: 주황색)
			// AmmoProgressBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.0f)); 
		}
		else if (bIsCharging)
		{
			// 충전 중
			AmmoProgressBar->SetPercent(ChargePercent);
			AmmoProgressBar->SetVisibility(ESlateVisibility::Visible);
			
			// 충전 중 색상 (예: 반투명 주황색 or 흰색)
		}
		else
		{
			// 비어있음 (대기)
			AmmoProgressBar->SetPercent(0.0f);
			// AmmoProgressBar->SetVisibility(ESlateVisibility::Hidden); // 숨기거나 빈 상태로 유지
			// 스타일 유지 위해 Hidden 대신 0%로 두는 것이 나을 수 있음 (배경이 보임)
		}
	}

	if (FullAmmoImage)
	{
		FullAmmoImage->SetVisibility(bIsFilled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UBrawlAmmoSlotWidget::PlayShakeAnimation()
{
	if (ShakeAnim)
	{
		PlayAnimation(ShakeAnim);
	}
}
