// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlPoisonZone.h"
#include "Components/StaticMeshComponent.h"

ABrawlPoisonZone::ABrawlPoisonZone()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	FogMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FogMesh"));
	FogMesh->SetupAttachment(RootComponent);
	FogMesh->SetCollisionProfileName(TEXT("NoCollision")); // 충돌 없음 (시각적 효과만)
	FogMesh->SetCastShadow(false); // 그림자 끄기
}

void ABrawlPoisonZone::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABrawlPoisonZone::SetZoneRadius(float NewRadius)
{
	CurrentRadius = NewRadius;

	// 메쉬 스케일 조정
	// 기본 Cylinder 반지름이 50이라고 가정하면, Scale = NewRadius / 50
	if (BaseMeshRadius > 0.0f)
	{
		float NewScale = CurrentRadius / BaseMeshRadius;
		
		// Z축(높이)은 적당히 크게 유지하거나, 반지름과 비례하지 않게 설정
		// 여기서는 거대한 벽을 만들기 위해 Z축도 키우거나 고정할 수 있음.
		// 일단은 XY 평면 스케일만 반지름에 맞추고, Z축은 높게 설정
		FogMesh->SetWorldScale3D(FVector(NewScale, NewScale, 20.0f)); 
	}
}
