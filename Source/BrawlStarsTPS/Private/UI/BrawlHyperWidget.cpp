// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHyperWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UBrawlHyperWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ImageHyperActive)
	{
		ImageHyperActive->SetVisibility(ESlateVisibility::Hidden);
	}

	if (ImageHyperReady)
	{
		ImageHyperReady->SetVisibility(ESlateVisibility::Hidden);
	}

	LastPercent = 0.0f;
	bIsActive = false;
}

void UBrawlHyperWidget::SetPercent(float InPercent)
{
	// 게이지 증가 시 플래시 효과 (Active 상태가 아닐 때만)
	if (!bIsActive && InPercent > LastPercent && InPercent < 1.0f)
	{
		OnChargeIncreased();
	}

	LastPercent = InPercent;
	Super::SetPercent(InPercent);
}

void UBrawlHyperWidget::SetIsReady(bool bNewIsReady)
{
	Super::SetIsReady(bNewIsReady);

	// Active 상태가 아닐 때만 Ready 이미지 표시 제어
	if (!bIsActive)
	{
		if (ImageHyperReady)
		{
			ImageHyperReady->SetVisibility(bNewIsReady ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		}
	}
}

void UBrawlHyperWidget::SetIsActive(bool bNewIsActive)
{
	bIsActive = bNewIsActive;

	if (bIsActive)
	{
		// 발동 상태: Ready와 일반 Progress 숨기고 Active 표시
		if (ImageHyperReady) ImageHyperReady->SetVisibility(ESlateVisibility::Hidden);
		if (ImageHyperActive) ImageHyperActive->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		// 발동 해제: 다시 Ready 상태나 일반 상태로 복귀
		if (ImageHyperActive) ImageHyperActive->SetVisibility(ESlateVisibility::Hidden);
		
		// 원래 상태 복원]
		SetIsReady(bIsReady); // bIsReady 값에 따라 Ready 이미지 가시성 재설정
	}
}

void UBrawlHyperWidget::SetActivePercent(float InPercent)
{
	SetPercent(InPercent);
}

void UBrawlHyperWidget::OnChargeIncreased()
{
	if (AnimFlash)
	{
		if (IsAnimationPlaying(AnimFlash))
		{
			StopAnimation(AnimFlash);
		}
		PlayAnimation(AnimFlash);
	}
}
