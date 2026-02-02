// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BrawlSpawnPoint.h"
#include "Components/BillboardComponent.h"
#include "UObject/ConstructorHelpers.h"

ABrawlSpawnPoint::ABrawlSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITOR
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	if (BillboardComponent)
	{
		RootComponent = BillboardComponent;
		
		// 에디터용 아이콘 설정 (기본 Target 아이콘 사용)
		static ConstructorHelpers::FObjectFinder<UTexture2D> IconTexture(TEXT("/Engine/EditorResources/S_Target.S_Target"));
		if (IconTexture.Succeeded())
		{
			BillboardComponent->SetSprite(IconTexture.Object);
		}
		
		BillboardComponent->bIsScreenSizeScaled = true;
	}
#else
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
#endif
}
