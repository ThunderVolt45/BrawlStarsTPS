// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlPawnComponent.h"

UBrawlPawnComponent::UBrawlPawnComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

APawn* UBrawlPawnComponent::GetPawn() const
{
	return Cast<APawn>(GetOwner());
}

AController* UBrawlPawnComponent::GetController() const
{
	if (APawn* Pawn = GetPawn())
	{
		return Pawn->GetController();
	}
	
	return nullptr;
}

