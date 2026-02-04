// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "BrawlPoisonZone.generated.h"

/**
 * ABrawlPoisonZone
 * 
 * 쇼다운 모드의 독구름(자기장)을 시각적으로 표현하는 액터입니다.
 * 사각형 형태의 안전 구역 정보를 머티리얼과 나이아가라에 전달합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlPoisonZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ABrawlPoisonZone();

protected:
	virtual void BeginPlay() override;

public:	
	// 현재 반지름(Half-Extent)을 설정하고 시각적 정보를 업데이트합니다.
	void SetZoneRadius(float NewRadius);

	// 현재 반지름 반환
	float GetZoneRadius() const { return CurrentRadius; }

	// 주어진 위치가 안전 구역 내부인지 확인합니다.
	UFUNCTION(BlueprintCallable, Category = "Brawl|Poison")
	bool IsPositionSafe(const FVector& InPosition) const;

protected:
	// 네비게이션 장애물로 사용할 4개의 박스 (상, 하, 좌, 우)
	UPROPERTY(VisibleAnywhere, Category = "Brawl|Navigation")
	TArray<TObjectPtr<UBoxComponent>> NavObstacles;

	// 네비게이션 업데이트를 위한 헬퍼 함수
	void UpdateNavObstacles();

	// 독구름을 표현할 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Visuals")
	TObjectPtr<UStaticMeshComponent> FogMesh;

	// 독구름을 표현할 파티클 (나이아가라)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Visuals")
	TObjectPtr<class UNiagaraComponent> FogParticle;

	// 현재 안전 구역의 반지름 (Half-Extent, Unreal Units)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Brawl|Poison")
	float CurrentRadius = 10000.0f;

	// 메쉬 기본 크기 보정값
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Config")
	float BaseMeshRadius = 50.0f;

	// 네비게이션 장애물이 실제 독구름보다 얼마나 더 안쪽으로 들어올지 결정하는 여유 거리
	UPROPERTY(EditAnywhere, Category = "Brawl|Config")
	float NavMargin = 150.0f;

	// 동적 머티리얼 인스턴스 (파라미터 전달용)
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> DynamicMaterial;
};