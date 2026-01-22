// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "BrawlProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class BRAWLSTARSTPS_API ABrawlProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABrawlProjectile();

	// 발사체 초기화 (어빌리티에서 호출)
	void InitializeProjectile(const FGameplayEffectSpecHandle& InDamageSpecHandle);

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// 충돌(Block) 시 호출
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 겹침(Overlap) 시 호출
	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 공통 처리 로직
	virtual void ProcessHit(AActor* OtherActor, const FVector& HitLocation);

protected:
	// 총알의 수명 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float LifeTime = 2.0f;
	
	// 총알의 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ProjectileSpeed = 3000.0f;
	
	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USphereComponent> SphereComponent;

	// 이동 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 외형 메쉬 (Blueprint에서 설정 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	// 적용할 데미지 Spec (GAS)
	FGameplayEffectSpecHandle DamageSpecHandle;

protected:
	// 관통 여부 (true면 적을 뚫고 지나감, false면 맞으면 사라짐)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Options")
	bool bCanPierce = false;

	// 장애물 파괴 여부 (true면 벽 파괴 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Options")
	bool bDestroyObstacles = false;

private:
	// 이미 피격된 액터 목록 (관통 시 다단히트 방지)
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HitActors;

	// 레이캐스트(Sweep)를 위한 이전 프레임 위치
	FVector PreviousLocation;
};
