// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlMatchStartWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UBrawlMatchStartWidget::SetupMatchInfo(FText ModeName, FText Description)
{
	if (TextBlock_ModeName) TextBlock_ModeName->SetText(ModeName);
	if (TextBlock_Description) TextBlock_Description->SetText(Description);

	// 로고는 처음엔 숨김
	if (Image_StartLogo) Image_StartLogo->SetVisibility(ESlateVisibility::Hidden);
}

void UBrawlMatchStartWidget::HideInfoText()
{
	if (TextBlock_ModeName) TextBlock_ModeName->SetVisibility(ESlateVisibility::Hidden);
	if (TextBlock_Description) TextBlock_Description->SetVisibility(ESlateVisibility::Hidden);
}

void UBrawlMatchStartWidget::PlayStartAnimation_Implementation()
{
	if (Image_StartLogo) Image_StartLogo->SetVisibility(ESlateVisibility::Visible);

	if (StartLogoAnim)
	{
		PlayAnimation(StartLogoAnim);
	}
}
