// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlUserWidget.h"
#include "Kismet/GameplayStatics.h"

void UBrawlUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}

void UBrawlUserWidget::PlayClickSFX()
{
	if (ClickSFX)
	{
		UGameplayStatics::PlaySound2D(this, ClickSFX);
	}
}
