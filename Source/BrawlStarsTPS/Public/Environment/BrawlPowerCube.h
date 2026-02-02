// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "BrawlPowerCube.generated.h"

class USphereComponent;
class URotatingMovementComponent;
class UGameplayEffect;

/**
 * ABrawlPowerCube
 * 
 * 획득 시 캐릭터의 파워 큐브 개수(공격력)와 최대 체력을 증가시키는 아이템입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlPowerCube : public AActor
{
	GENERATED_BODY()
	
public:	
	ABrawlPowerCube();

protected:
	virtual void BeginPlay() override;

	// 오버랩 처리
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CubeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URotatingMovementComponent> RotatingMovement;

	// 획득 시 적용할 이펙트 (GE_PowerCube)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|PowerCube")
	TSubclassOf<UGameplayEffect> PowerCubeEffectClass;

	// 획득 효과음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|PowerCube")
	TObjectPtr<USoundBase> PickupSound;

	// 등장 효과음 (스폰 시 재생)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|PowerCube")
	TObjectPtr<USoundBase> SpawnSound;

	// 획득 시 파티클 효과
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|PowerCube")
	TObjectPtr<UParticleSystem> PickupVFX;
};
