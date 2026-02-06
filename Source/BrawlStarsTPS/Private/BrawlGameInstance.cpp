// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlGameInstance.h"
#include "Kismet/GameplayStatics.h"

UBrawlGameInstance::UBrawlGameInstance()
{
}

void UBrawlGameInstance::StartGame()
{
	if (SelectedMapName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlGameInstance: SelectedMapName is None! Cannot start game."));
		return;
	}

	UGameplayStatics::OpenLevel(this, SelectedMapName);
}
