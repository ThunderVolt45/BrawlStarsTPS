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

	// 네비게이션 장애물용 박스 4개 생성
	for (int32 i = 0; i < 4; ++i)
	{
		FName BoxName = *FString::Printf(TEXT("NavBox_%d"), i);
		UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(BoxName);
		Box->SetupAttachment(RootComponent);
		
		// 충돌은 끄고 네비게이션에만 영향을 주도록 설정
		Box->SetCollisionProfileName(TEXT("NoCollision"));
		Box->SetCanEverAffectNavigation(true);
		
		// 시각화 설정: 에디터와 게임 내에서 빨간색으로 표시
		Box->bHiddenInGame = false;
		Box->ShapeColor = FColor::Red;
		Box->SetLineThickness(3.0f);
		
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

	UpdateNavObstacles();
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

	// 4. 네비게이션 장애물 위치 업데이트
	UpdateNavObstacles();

	// 5. 기본적인 스케일 조정 (백업용)
	if (BaseMeshRadius > 0.0f)
	{
		float NewScale = CurrentRadius / BaseMeshRadius;
		FogMesh->SetWorldScale3D(FVector(NewScale, NewScale, 1.0f));
		FogParticle->SetWorldScale3D(FVector(NewScale, NewScale, 1.0f));
	}
}

void ABrawlPoisonZone::UpdateNavObstacles()
{
	// 맵 전체를 덮을 만큼 충분히 큰 값 (필요 시 더 크게 조정)
	const float MaxMapExtent = 15000.0f; 
	const float Thickness = 5000.0f; // 독구름 영역의 두께
	const float HalfThickness = Thickness * 0.5f;
	const float Height = 1000.0f;

	// 실제 독구름보다 마진(NavMargin)만큼 안쪽으로 장애물 설정
	const float EffectiveRadius = FMath::Max(0.0f, CurrentRadius - NavMargin);

	// 안전 구역 바깥쪽 4면을 감싸는 박스 좌표 및 크기 계산
	// 0: +X (위쪽), 1: -X (아래쪽), 2: -Y (왼쪽), 3: +Y (오른쪽)
	
	// 각 박스의 중심 오프셋
	FVector Offsets[4] = {
		FVector(EffectiveRadius + HalfThickness, 0.0f, 0.0f),
		FVector(-(EffectiveRadius + HalfThickness), 0.0f, 0.0f),
		FVector(0.0f, -(EffectiveRadius + HalfThickness), 0.0f),
		FVector(0.0f, EffectiveRadius + HalfThickness, 0.0f)
	};

	// 각 박스의 범위 (Half-Extent)
	FVector Extents[4] = {
		FVector(HalfThickness, MaxMapExtent, Height), // 상
		FVector(HalfThickness, MaxMapExtent, Height), // 하
		FVector(EffectiveRadius, HalfThickness, Height), // 좌
		FVector(EffectiveRadius, HalfThickness, Height)  // 우
	};

	for (int32 i = 0; i < 4; ++i)
	{
		if (NavObstacles.IsValidIndex(i) && NavObstacles[i])
		{
			NavObstacles[i]->SetRelativeLocation(Offsets[i]);
			NavObstacles[i]->SetBoxExtent(Extents[i]);
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
