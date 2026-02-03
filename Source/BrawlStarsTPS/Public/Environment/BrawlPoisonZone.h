// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

protected:
	// 독구름을 표현할 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> FogMesh;

	// 독구름을 표현할 파티클 (나이아가라)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<class UNiagaraComponent> FogParticle;

	// 현재 안전 구역의 반지름 (Half-Extent, Unreal Units)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Poison")
	float CurrentRadius = 10000.0f;

	// 메쉬 기본 크기 보정값
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float BaseMeshRadius = 50.0f;

	// 동적 머티리얼 인스턴스 (파라미터 전달용)
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> DynamicMaterial;
};