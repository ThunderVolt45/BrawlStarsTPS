// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlUserWidget.h"

void UBrawlUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
