// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Environment/BrawlDestructibleInterface.h"
#include "BrawlObstacle.generated.h"

class USoundBase;

/**
 * ABrawlObstacle
 * 
 * 게임 내 장애물(벽, 상자 등)의 기본 클래스
 * IBrawlDestructibleInterface를 구현하여 특수 공격 등에 의해 파괴될 수 있음
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlObstacle : public AActor, public IBrawlDestructibleInterface
{
	GENERATED_BODY()
	
public:	
	ABrawlObstacle();

	virtual void OnConstruction(const FTransform& Transform) override;

	// IBrawlDestructibleInterface
	virtual bool IsDestructible() const override;
	virtual void OnDestruction(AActor* InstigatorActor) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacle")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 게임 시작 시 이 목록 중 하나를 랜덤으로 선택하여 메시를 변경합니다.
	// 비어있으면 기본 설정된 메시를 사용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle|Visuals")
	TArray<TObjectPtr<UStaticMesh>> RandomMeshes;

	// 파괴 가능한지 여부 (Blueprint에서 설정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle")
	bool bIsDestructible = true;

	// 파괴 시 그 자리에 생성할 액터 (예: Chaos Geometry Collection, 파편 액터)
	// 평소에는 가벼운 StaticMesh를 쓰다가, 파괴 순간에만 이 액터로 교체합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle|FX")
	TSubclassOf<AActor> DestructionEffectClass;

	// 파괴 효과음
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle|FX")
	TObjectPtr<USoundBase> DestructionSFX;
};
