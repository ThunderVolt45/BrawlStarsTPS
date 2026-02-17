// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "BrawlPoolableInterface.h"
#include "BrawlPowerCube.generated.h"

class USphereComponent;
class URotatingMovementComponent;
class UProjectileMovementComponent;
class UGameplayEffect;

/**
 * ABrawlPowerCube
 * 
 * 획득 시 캐릭터의 파워 큐브 개수(공격력)와 최대 체력을 증가시키는 아이템입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlPowerCube : public AActor, public IBrawlPoolableInterface
{
	GENERATED_BODY()
	
public:	
	ABrawlPowerCube();

	// --- IBrawlPoolableInterface 구현 ---
	virtual void OnActivate() override;
	virtual void OnDeactivate() override;
	virtual bool IsActive() const override { return bIsActive; }
	virtual void GetPrewarmRequirements(TMap<TSubclassOf<AActor>, int32>& OutRequirements, int32 BaseCount) const override;
	virtual void GetGameplayCueTags(FGameplayTagContainer& OutTags) const;
	// ----------------------------------

protected:
	virtual void BeginPlay() override;

	/** 풀로 반환 */
	void Deactivate();

	// 오버랩 처리
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CubeMesh;

	/** 지형 충돌용 작은 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	/** 캐릭터 획득 감지용 큰 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URotatingMovementComponent> RotatingMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// AI 감지용 소스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|AI")
	TObjectPtr<class UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;

	// 획득 시 적용할 이펙트 (GE_PowerCube)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|PowerCube")
	TSubclassOf<UGameplayEffect> PowerCubeEffectClass;

	// 획득 시 실행할 GameplayCue 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|PowerCube")
	FGameplayTag PickupCueTag;

	// 등장 시 실행할 GameplayCue 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|PowerCube")
	FGameplayTag SpawnCueTag;

private:
	bool bIsActive = false;
};
