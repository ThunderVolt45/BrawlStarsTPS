// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlProjectile.h"
#include "BrawlProjectile_Spawner.generated.h"

/**
 * ABrawlProjectile_Spawner
 * 
 * 착탄(OnHit) 시 지정된 액터(장판, 터렛 등)를 스폰하고 사라지는 발사체입니다.
 * 예: 스파이크의 선인장 수류탄 (착탄 시 슬로우 장판 생성)
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlProjectile_Spawner : public ABrawlProjectile
{
	GENERATED_BODY()
	
public:
	// 착탄 시 처리 오버라이드
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	// 스폰할 액터 클래스 (예: BP_Spike_Super_Area)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<AActor> ActorClassToSpawn;

	// 스폰 시 Z축 오프셋 (바닥에 파묻히지 않게 조정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawner")
	float SpawnZOffset = 0.0f;
};
