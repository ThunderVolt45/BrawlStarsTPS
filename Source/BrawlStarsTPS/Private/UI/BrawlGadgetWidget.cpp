// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlGadgetWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBrawlGadgetWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Active 이미지의 다이내믹 머티리얼 생성
	if (ImageProgress)
	{
		if (ImageProgress->GetBrush().GetResourceObject())
		{
			ActiveMaterialDynamic = ImageProgress->GetDynamicMaterial();
			
			if (ActiveMaterialDynamic)
			{
				ActiveMaterialDynamic->SetTextureParameterValue(MaterialMaskTextureParameterName, TextureCooldownMask);
			}
		}
	}
	
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
			TextCooldown->SetText(FText::AsNumber(FMath::CeilToFloat(RemainingTime * 10.f) / 10.f));
			
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
