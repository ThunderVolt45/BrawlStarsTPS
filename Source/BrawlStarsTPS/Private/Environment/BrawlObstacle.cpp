// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlObstacle.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

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

void ABrawlObstacle::OnDestruction(AActor* InstigatorActor)
{
	if (!bIsDestructible) return;

	// 1. 파괴된 형상(액터) 스폰 (교체)
	if (DestructionEffectClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// 원본과 동일한 위치/회전/스케일에 스폰
		GetWorld()->SpawnActor<AActor>(DestructionEffectClass, GetActorTransform(), SpawnParams);
	}

	// 2. 효과음 재생
	if (DestructionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestructionSFX, GetActorLocation());
	}
	
	// 3. 원본 삭제
	Destroy();
}
