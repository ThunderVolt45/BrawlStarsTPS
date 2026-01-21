// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlGadgetWidget.h"
#include "Components/TextBlock.h"

void UBrawlGadgetWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Text_Cooldown)
	{
		Text_Cooldown->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UBrawlGadgetWidget::SetRemainingCooldown(float RemainingTime)
{
	if (Text_Cooldown)
	{
		if (RemainingTime > 0.f)
		{
			// 소수점 첫째 자리까지 표시 (예: 1.5)
			Text_Cooldown->SetText(FText::AsNumber(FMath::CeilToInt(RemainingTime)));
			
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

	if (Text_Cooldown)
	{
		// 준비 완료되면 시간을 숨기고, 쿨다운 중이면 보여줌
		Text_Cooldown->SetVisibility(bNewIsReady ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
}
