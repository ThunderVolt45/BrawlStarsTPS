// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BrawlTieBreaker.generated.h"

UCLASS()
class BRAWLSTARSTPS_API ABrawlTieBreaker : public AActor
{
	GENERATED_BODY()
	
public:	
	ABrawlTieBreaker();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Brawl|Components")
	TObjectPtr<class USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Brawl|Components")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	// 회전 효과 등...
	virtual void Tick(float DeltaTime) override;
};
