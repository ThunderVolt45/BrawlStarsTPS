// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlGadgetWidget.h"
#include "Components/TextBlock.h"

void UBrawlGadgetWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TextCooldown)
	{
		TextCooldown->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UBrawlGadgetWidget::SetRemainingCooldown(float RemainingTime)
{
	if (TextCooldown)
	{
		if (RemainingTime > 0.f)
		{
			// 소수점 첫째 자리까지 표시 (예: 1.5)
			TextCooldown->SetText(FText::AsNumber(FMath::CeilToInt(RemainingTime)));
			
			if (bIsReady)
			{
				SetIsReady(false);
			}
		}
		else
		{
			if (!bIsReady)
			{
				SetIsReady(true);
			}
		}
	}
}

void UBrawlGadgetWidget::SetIsReady(bool bNewIsReady)
{
	Super::SetIsReady(bNewIsReady);

	if (TextCooldown)
	{
		// 준비 완료되면 시간을 숨기고, 쿨다운 중이면 보여줌
		TextCooldown->SetVisibility(bNewIsReady ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
}
