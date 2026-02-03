// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlPoisonZone.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"

ABrawlPoisonZone::ABrawlPoisonZone()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	FogMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FogMesh"));
	FogMesh->SetupAttachment(RootComponent);
	FogMesh->SetCollisionProfileName(TEXT("NoCollision"));
	FogMesh->SetCastShadow(false);

	FogParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogParticle"));
	FogParticle->SetupAttachment(RootComponent);
	FogParticle->SetAutoActivate(true);
}

void ABrawlPoisonZone::BeginPlay()
{
	Super::BeginPlay();
	
	// 머티리얼 파라미터 조정을 위한 동적 머티리얼 생성
	if (FogMesh && FogMesh->GetMaterial(0))
	{
		DynamicMaterial = FogMesh->CreateDynamicMaterialInstance(0);
	}
}

void ABrawlPoisonZone::SetZoneRadius(float NewRadius)
{
	CurrentRadius = NewRadius;

	// 1. 디버그 드로잉 (안전지대 경계 표시 - 사각형)
	// 매 업데이트마다 0.2초간 유지되는 박스를 그려 현재 범위를 보여줌
	DrawDebugBox(GetWorld(), GetActorLocation(), FVector(CurrentRadius, CurrentRadius, 500.0f), FColor::Green, false, 0.2f, 0, 10.0f);

	// 2. 머티리얼 파라미터 업데이트
	// 추후 만드실 이펙트 머티리얼에서 'SafeZoneRadius'라는 이름의 Scalar Parameter를 쓰시면 됩니다.
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("SafeZoneRadius"), CurrentRadius);
	}

	// 3. 나이아가라 파라미터 업데이트
	// 나이아가라 시스템에서도 'Radius' 파라미터를 사용해 사각형 입자 배치를 조절할 수 있습니다.
	if (FogParticle)
	{
		FogParticle->SetVariableFloat(TEXT("Radius"), CurrentRadius);
	}

	// 4. 기본적인 스케일 조정 (백업용)
	if (BaseMeshRadius > 0.0f)
	{
		float NewScale = CurrentRadius / BaseMeshRadius;
		FogMesh->SetWorldScale3D(FVector(NewScale, NewScale, 1.0f));
		FogParticle->SetWorldScale3D(FVector(NewScale, NewScale, 1.0f));
	}
}
