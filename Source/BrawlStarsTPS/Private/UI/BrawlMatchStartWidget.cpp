// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlMatchStartWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UBrawlMatchStartWidget::SetupMatchInfo(FText ModeName, FText Description)
{
	if (TextModeName) TextModeName->SetText(ModeName);
	if (TextDescription) TextDescription->SetText(Description);

	// 로고는 처음엔 숨김 및 초기화
	if (ImageStartLogo)
	{
		ImageStartLogo->SetVisibility(ESlateVisibility::Hidden);
		ImageStartLogo->SetRenderScale(FVector2D(0.5f, 0.5f));
	}
}

void UBrawlMatchStartWidget::HideInfoText()
{
	if (TextModeName) TextModeName->SetVisibility(ESlateVisibility::Hidden);
	if (TextDescription) TextDescription->SetVisibility(ESlateVisibility::Hidden);
}

void UBrawlMatchStartWidget::PlayStartAnimation_Implementation()
{
	if (ImageStartLogo)
	{
		ImageStartLogo->SetVisibility(ESlateVisibility::Visible);
		bIsAnimatingLogo = true;
		AnimationProgress = 0.0f;
	}
}

void UBrawlMatchStartWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsAnimatingLogo && ImageStartLogo)
	{
		AnimationProgress += InDeltaTime / AnimationDuration;

		if (AnimationProgress >= 1.0f)
		{
			AnimationProgress = 1.0f;
			bIsAnimatingLogo = false;
		}

		// 0.5에서 1.0까지 확대되는 효과
		float CurrentScale = FMath::Lerp(0.5f, 1.0f, AnimationProgress);
		ImageStartLogo->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
	}
}
