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

	// 바닥 독구름용 기본 평면 메쉬 설정
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		FogMesh->SetStaticMesh(PlaneMesh.Object);
	}

	FogParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogParticle"));
	FogParticle->SetupAttachment(RootComponent);
	FogParticle->SetAutoActivate(true);

	// 독구름 벽면 메쉬 4개 생성 (시각 효과 전용)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WallMesh(TEXT("/Engine/BasicShapes/Plane"));
	
	for (int32 i = 0; i < 4; ++i)
	{
		FName MeshName = *FString::Printf(TEXT("FogWallMesh_%d"), i);
		UStaticMeshComponent* Wall = CreateDefaultSubobject<UStaticMeshComponent>(MeshName);
		Wall->SetupAttachment(RootComponent);
		
		if (WallMesh.Succeeded())
		{
			Wall->SetStaticMesh(WallMesh.Object);
		}

		// 벽면 회전 설정 (안쪽을 바라보도록)
		FRotator WallRotation(0.f, 0.f, 0.f);
		if (i == 0) WallRotation = FRotator(90.f, 0.f, 180.f); // +X 벽
		else if (i == 1) WallRotation = FRotator(90.f, 0.f, 0.f);    // -X 벽
		else if (i == 2) WallRotation = FRotator(90.f, 0.f, -90.f); // -Y 벽
		else if (i == 3) WallRotation = FRotator(90.f, 0.f, 90.f);  // +Y 벽
		
		Wall->SetRelativeRotation(WallRotation);
		Wall->SetCollisionProfileName(TEXT("NoCollision"));
		Wall->SetCanEverAffectNavigation(false); // 시각 메쉬는 네비게이션 영향 끔
		Wall->SetCastShadow(false);
		
		FogWallMeshes.Add(Wall);
	}

	// 네비게이션 장애물 전용 박스 생성
	for (int32 i = 0; i < 4; ++i)
	{
		FName BoxName = *FString::Printf(TEXT("NavBox_%d"), i);
		UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(BoxName);
		Box->SetupAttachment(RootComponent);
		
		Box->SetCollisionProfileName(TEXT("NoCollision"));
		Box->SetCanEverAffectNavigation(true);
		Box->bHiddenInGame = true;
		
		NavObstacles.Add(Box);
	}
}

void ABrawlPoisonZone::BeginPlay()
{
	Super::BeginPlay();
	
	// 머티리얼 파라미터 조정을 위한 동적 머티리얼 생성
	if (FogMesh && FogMesh->GetMaterial(0))
	{
		DynamicMaterial = FogMesh->CreateDynamicMaterialInstance(0);
	}

	// 각 벽면 메쉬에도 머티리얼 적용 (필요 시 개별 DynamicMaterial 생성 가능)
	for (UStaticMeshComponent* Wall : FogWallMeshes)
	{
		if (Wall && DynamicMaterial)
		{
			Wall->SetMaterial(0, DynamicMaterial);
		}
	}

	UpdateFogWalls();
}

void ABrawlPoisonZone::SetZoneRadius(float NewRadius)
{
	CurrentRadius = NewRadius;

	// 1. 디버그 드로잉 (안전지대 경계 표시 - 사각형)
	// DrawDebugBox(GetWorld(), GetActorLocation(), FVector(CurrentRadius, CurrentRadius, 500.0f), FColor::Green, false, 0.2f, 0, 10.0f);

	// 2. 머티리얼 파라미터 업데이트
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("SafeZoneRadius"), CurrentRadius);
	}

	// 3. 나이아가라 파라미터 업데이트
	if (FogParticle)
	{
		FogParticle->SetVariableFloat(TEXT("Radius"), CurrentRadius);
	}

	// 4. 벽면 메쉬 및 네비게이션 업데이트
	UpdateFogWalls();

	// 5. 기본적인 스케일 조정 (바닥 메쉬용)
	if (BaseMeshRadius > 0.0f)
	{
		float NewScale = CurrentRadius / BaseMeshRadius;
		FogMesh->SetWorldScale3D(FVector(NewScale, NewScale, 1.0f));
	}
}

void ABrawlPoisonZone::UpdateFogWalls()
{
	// 맵 전체를 덮을 만큼 충분히 큰 값
	const float MaxMapExtent = 15000.0f; 
	const float Thickness = 5000.0f; // 네비게이션 장애물 두께
	const float HalfThickness = Thickness * 0.5f;
	const float Height = 1000.0f;
	
	const float PlaneSize = 100.0f;

	// 실제 독구름보다 마진(NavMargin)만큼 안쪽으로 장애물 설정
	const float EffectiveRadius = FMath::Max(0.0f, CurrentRadius - NavMargin);

	// 0: +X, 1: -X, 2: -Y, 3: +Y

	// 1. 시각용 평면(Plane) 위치 및 스케일 업데이트
	// 시각적으로는 딱 CurrentRadius 위치에 배치
	FVector VisualOffsets[4] = {
		FVector(CurrentRadius, 0.0f, Height * 0.5f),
		FVector(-CurrentRadius, 0.0f, Height * 0.5f),
		FVector(0.0f, -CurrentRadius, Height * 0.5f),
		FVector(0.0f, CurrentRadius, Height * 0.5f)
	};

	for (int32 i = 0; i < 4; ++i)
	{
		if (FogWallMeshes.IsValidIndex(i) && FogWallMeshes[i])
		{
			FogWallMeshes[i]->SetRelativeLocation(VisualOffsets[i]);
			// 가로 너비를 딱 안전 구역의 한 변의 길이(CurrentRadius * 2)로 설정
			float WallWidth = CurrentRadius * 2.0f;
			FogWallMeshes[i]->SetRelativeScale3D(FVector(Height / PlaneSize, WallWidth / PlaneSize, 1.0f));
		}
	}

	// 2. 네비게이션용 박스(Box) 위치 및 크기 업데이트
	// 두께(Thickness)를 고려하여 바깥쪽으로 배치
	FVector NavOffsets[4] = {
		FVector(EffectiveRadius + HalfThickness, 0.0f, Height * 0.5f),
		FVector(-(EffectiveRadius + HalfThickness), 0.0f, Height * 0.5f),
		FVector(0.0f, -(EffectiveRadius + HalfThickness), Height * 0.5f),
		FVector(0.0f, EffectiveRadius + HalfThickness, Height * 0.5f)
	};

	FVector NavExtents[4] = {
		FVector(HalfThickness, MaxMapExtent, Height * 0.5f),
		FVector(HalfThickness, MaxMapExtent, Height * 0.5f),
		FVector(EffectiveRadius, HalfThickness, Height * 0.5f),
		FVector(EffectiveRadius, HalfThickness, Height * 0.5f)
	};

	for (int32 i = 0; i < 4; ++i)
	{
		if (NavObstacles.IsValidIndex(i) && NavObstacles[i])
		{
			NavObstacles[i]->SetRelativeLocation(NavOffsets[i]);
			NavObstacles[i]->SetBoxExtent(NavExtents[i]);
		}
	}
}

bool ABrawlPoisonZone::IsPositionSafe(const FVector& InPosition) const
{
	FVector ZoneCenter = GetActorLocation();
	
	// 2D 사각형 체크 (X, Y 좌표가 반지름 내에 있는지)
	bool bInX = FMath::Abs(InPosition.X - ZoneCenter.X) <= CurrentRadius;
	bool bInY = FMath::Abs(InPosition.Y - ZoneCenter.Y) <= CurrentRadius;

	return bInX && bInY;
}

FVector ABrawlPoisonZone::GetClosestSafePosition(const FVector& InPosition) const
{
	FVector ZoneCenter = GetActorLocation();
	// NavMargin만큼 더 안쪽으로 고정하여 확실한 안전 확보
	float SafeRadius = FMath::Max(0.0f, CurrentRadius - NavMargin);

	FVector ClampedPos = InPosition;
	ClampedPos.X = FMath::Clamp(InPosition.X, ZoneCenter.X - SafeRadius, ZoneCenter.X + SafeRadius);
	ClampedPos.Y = FMath::Clamp(InPosition.Y, ZoneCenter.Y - SafeRadius, ZoneCenter.Y + SafeRadius);

	return ClampedPos;
}
