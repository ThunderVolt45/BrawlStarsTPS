#include "Environment/BrawlObstacle.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

ABrawlObstacle::ABrawlObstacle()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 멀티플레이어 고려 시 필요

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	
	// 기본적으로 블록 설정
	MeshComponent->SetCollisionProfileName(FName("BlockAll"));
}

void ABrawlObstacle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 랜덤 메시 적용 (에디터에서도 확인 가능)
	if (RandomMeshes.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, RandomMeshes.Num() - 1);
		
		if (UStaticMesh* SelectedMesh = RandomMeshes[RandomIndex])
		{
			MeshComponent->SetStaticMesh(SelectedMesh);
		}
	}
}

bool ABrawlObstacle::IsDestructible() const
{
	return bIsDestructible;
}

bool ABrawlObstacle::IsHardObstacle() const
{
	return bIsHardObstacle;
}

void ABrawlObstacle::OnDestruction(AActor* InstigatorActor)
{
	if (!bIsDestructible) return;

	// 1. 파괴된 형상(액터) 스폰 (교체)
	if (DestructionEffectClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// 원본과 동일한 위치/회전/스케일에 스폰
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(DestructionEffectClass, GetActorTransform(), SpawnParams);

		if (SpawnedActor)
		{
			// 지오메트리 컬렉션 컴포넌트를 찾아 물리적 충격 가하기 (즉시 파괴 연출)
			if (UGeometryCollectionComponent* GCComp = SpawnedActor->FindComponentByClass<UGeometryCollectionComponent>())
			{
				// 공격자 방향에서 밀려나도록 위치 설정 (공격자가 없으면 중앙 하단에서 폭발)
				FVector ImpulseOrigin = InstigatorActor ? 
					GetActorLocation() + (InstigatorActor->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 50.0f : 
					GetActorLocation() - FVector(0, 0, 50.0f);
				
				GCComp->AddRadialImpulse(ImpulseOrigin, ImpulseRadius, ImpulseStrength, ERadialImpulseFalloff::RIF_Linear, true);
			}
		}
	}

	// 2. 효과음 재생
	if (DestructionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestructionSFX, GetActorLocation());
	}
	
	// 3. 원본 삭제
	Destroy();
}
