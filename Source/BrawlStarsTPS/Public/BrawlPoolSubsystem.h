// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BrawlPoolSubsystem.generated.h"

USTRUCT()
struct FBrawlActorPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> InactiveActors;
};

/**
 * 범용 오브젝트 풀링 서브시스템
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 풀에서 액터를 가져오거나 없으면 새로 생성합니다. */
	AActor* GetFromPool(TSubclassOf<AActor> ActorClass, const FTransform& Transform, AActor* Owner = nullptr, APawn* Instigator = nullptr, TFunction<void(AActor*)> PreActivateFunc = nullptr);

	/** 액터를 풀로 반환합니다. */
	void ReturnToPool(AActor* Actor);

	/** 특정 클래스의 풀을 미리 생성해둡니다. (최적화용) */
	void PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count);

private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FBrawlActorPool> PoolMap;
};
