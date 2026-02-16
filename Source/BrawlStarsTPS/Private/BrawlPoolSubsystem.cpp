// Fill out your copyright notice in the Description page of Project Settings.

#include "BrawlPoolSubsystem.h"
#include "BrawlPoolableInterface.h"

AActor* UBrawlPoolSubsystem::GetFromPool(TSubclassOf<AActor> ActorClass, const FTransform& Transform, AActor* Owner, APawn* Instigator)
{
	if (!ActorClass) return nullptr;

	FBrawlActorPool& Pool = PoolMap.FindOrAdd(ActorClass);
	AActor* Actor = nullptr;

	// 1. 풀에 유효한 액터가 있는지 확인
	while (Pool.InactiveActors.Num() > 0)
	{
		Actor = Pool.InactiveActors.Pop();
		if (IsValid(Actor)) break;
		Actor = nullptr;
	}

	// 2. 풀에 없으면 새로 생성
	if (!Actor)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.Instigator = Instigator;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Actor = GetWorld()->SpawnActor<AActor>(ActorClass, Transform, SpawnParams);
	}
	else
	{
		// 풀에서 꺼낸 경우 정보 업데이트
		Actor->SetOwner(Owner);
		Actor->SetInstigator(Instigator);
		Actor->SetActorTransform(Transform);
	}

	if (Actor)
	{
		// 3. 인터페이스가 있으면 OnActivate 호출, 없으면 기본 활성화
		if (IBrawlPoolableInterface* Poolable = Cast<IBrawlPoolableInterface>(Actor))
		{
			Poolable->OnActivate();
		}
		else
		{
			Actor->SetActorHiddenInGame(false);
			Actor->SetActorEnableCollision(true);
			Actor->SetActorTickEnabled(true);
		}
	}

	return Actor;
}

void UBrawlPoolSubsystem::ReturnToPool(AActor* Actor)
{
	if (!IsValid(Actor)) return;

	// 1. 인터페이스가 있으면 OnDeactivate 호출, 없으면 기본 비활성화
	if (IBrawlPoolableInterface* Poolable = Cast<IBrawlPoolableInterface>(Actor))
	{
		Poolable->OnDeactivate();
	}
	else
	{
		Actor->SetActorHiddenInGame(true);
		Actor->SetActorEnableCollision(false);
		Actor->SetActorTickEnabled(false);
	}

	// 2. 풀에 추가
	PoolMap.FindOrAdd(Actor->GetClass()).InactiveActors.Add(Actor);
}

void UBrawlPoolSubsystem::PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!ActorClass) return;

	for (int32 i = 0; i < Count; ++i)
	{
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity);
		if (NewActor)
		{
			ReturnToPool(NewActor);
		}
	}
}
