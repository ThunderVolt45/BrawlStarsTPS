// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BrawlPoisonZone.generated.h"

/**
 * ABrawlPoisonZone
 * 
 * 쇼다운 모드의 독구름(자기장)을 시각적으로 표현하는 액터입니다.
 * 실제 데미지 판정은 GameMode에서 수행하지만, 시각적인 범위 표시는 이 액터가 담당합니다.
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
	// 현재 반지름을 설정하고 시각적 크기를 업데이트합니다.
	void SetZoneRadius(float NewRadius);

	// 현재 반지름 반환
	float GetZoneRadius() const { return CurrentRadius; }

protected:
	// 독구름을 표현할 메쉬 (원통형 예상)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> FogMesh;

	// 현재 안전 구역의 반지름 (Unreal Units)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Poison")
	float CurrentRadius = 10000.0f;

	// 메쉬의 기본 크기 (반지름 50 단위의 원통을 가정했을 때 보정값)
	// 예: 언리얼 기본 Cylinder는 반지름 50, 높이 200 등. 
	// 스케일 1.0 = 반지름 50 -> 반지름 1000을 만들려면 스케일 20.
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float BaseMeshRadius = 50.0f;
};
