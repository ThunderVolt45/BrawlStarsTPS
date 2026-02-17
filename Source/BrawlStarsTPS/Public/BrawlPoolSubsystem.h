// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
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

	/** 레벨에 배치된 환경 오브젝트(장애물, 상자 등)들을 스캔하여 필요한 클래스들을 미리 생성합니다. */
	void PrewarmEnvironmentActors();

	/** 지정된 태그 목록에 해당하는 GameplayCue 에셋들을 미리 로드합니다. */
	void PrewarmGameplayCues(const FGameplayTagContainer& CueTags);

private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FBrawlActorPool> PoolMap;

	/** 이미 사전 로드(Preload) 또는 프리워밍이 완료된 클래스 목록 */
	UPROPERTY()
	TSet<TSubclassOf<AActor>> PreloadedClasses;

	/** 이미 사전 로드된 GameplayCue 태그 목록 */
	UPROPERTY()
	TSet<FGameplayTag> PreloadedTags;
};
