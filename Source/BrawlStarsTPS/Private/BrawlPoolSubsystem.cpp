// Fill out your copyright notice in the Description page of Project Settings.

#include "BrawlPoolSubsystem.h"
#include "BrawlPoolableInterface.h"
#include "EngineUtils.h"
#include "AbilitySystemGlobals.h"
#include "BrawlProjectile.h"
#include "GameplayCueManager.h"
#include "Environment/BrawlPowerCube.h"

AActor* UBrawlPoolSubsystem::GetFromPool(TSubclassOf<AActor> ActorClass, const FTransform& Transform, AActor* Owner, APawn* Instigator, TFunction<void(AActor*)> PreActivateFunc)
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
		
		// 충돌을 무시하고 강제로 위치를 이동시키기 위해 TeleportPhysics 사용
		Actor->SetActorLocationAndRotation(Transform.GetLocation(), Transform.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
		Actor->SetActorScale3D(Transform.GetScale3D());
	}

	if (Actor)
	{
		// 3. 활성화 전 초기화 콜백 실행 (데미지 설정 등)
		if (PreActivateFunc)
		{
			PreActivateFunc(Actor);
		}

		// 4. 인터페이스가 있으면 OnActivate 호출, 없으면 기본 활성화
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
	if (!ActorClass || Count <= 0) return;

	// 인터페이스 구현 여부 확인
	bool bIsPoolable = ActorClass->ImplementsInterface(UBrawlPoolableInterface::StaticClass());

	// 1. 풀링이 불가능한 클래스 (예: GeometryCollectionActor) 처리
	if (!bIsPoolable)
	{
		// 이미 한 번이라도 로드(스폰)되었다면 다시 하지 않음
		if (PreloadedClasses.Contains(ActorClass)) return;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AActor* TempActor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity, SpawnParams))
		{
			TempActor->Destroy();
			PreloadedClasses.Add(ActorClass);
			UE_LOG(LogTemp, Log, TEXT("PoolSubsystem: Successfully Pre-loaded Non-Poolable Class [%s]"), *ActorClass->GetName());
		}
		return;
	}

	// 2. 풀링 가능한 클래스 처리 (브롤러별/오브젝트별 요청 개수만큼 누적 생성)
	for (int32 i = 0; i < Count; ++i)
	{
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity);
		if (NewActor)
		{
			ReturnToPool(NewActor);
		}
	}

	// 로드 완료 표시 (풀링 가능 클래스도 관리 대상에 포함)
	PreloadedClasses.Add(ActorClass);
}

void UBrawlPoolSubsystem::PrewarmEnvironmentActors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TMap<TSubclassOf<AActor>, int32> TotalRequirements;
	FGameplayTagContainer TotalCueTags;

	// 재귀적으로 요구사항을 수집하는 헬퍼 람다
	TFunction<void(TSubclassOf<AActor>, int32)> ProcessClass;
	ProcessClass = [&](TSubclassOf<AActor> ActorClass, int32 Count)
	{
		if (!ActorClass) return;

		TotalRequirements.FindOrAdd(ActorClass) += Count;

		if (AActor* CDO = Cast<AActor>(ActorClass->GetDefaultObject()))
		{
			// 1. 발사체나 파워큐브 등이라면 태그 수집
			if (ABrawlProjectile* ProjectileCDO = Cast<ABrawlProjectile>(CDO))
			{
				ProjectileCDO->GetGameplayCueTags(TotalCueTags);
			}
			else if (ABrawlPowerCube* PowerCubeCDO = Cast<ABrawlPowerCube>(CDO))
			{
				PowerCubeCDO->GetGameplayCueTags(TotalCueTags);
			}

			// 2. 풀링 인터페이스를 통해 하위 요구사항 확인
			if (IBrawlPoolableInterface* Poolable = Cast<IBrawlPoolableInterface>(CDO))
			{
				TMap<TSubclassOf<AActor>, int32> SubRequirements;
				Poolable->GetPrewarmRequirements(SubRequirements, Count);
				for (auto& Pair : SubRequirements)
				{
					ProcessClass(Pair.Key, Pair.Value);
				}
			}
		}
	};

	// 레벨에 배치된 모든 액터 중 풀링 인터페이스를 구현한 액터 스캔
	for (FActorIterator It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor))
		{
			if (IBrawlPoolableInterface* Poolable = Cast<IBrawlPoolableInterface>(Actor))
			{
				// 레벨 배치 액터 자신의 태그 수집
				if (ABrawlPowerCube* PowerCube = Cast<ABrawlPowerCube>(Actor))
				{
					PowerCube->GetGameplayCueTags(TotalCueTags);
				}

				// 하위 요구사항들 수집
				TMap<TSubclassOf<AActor>, int32> SubRequirements;
				Poolable->GetPrewarmRequirements(SubRequirements, 1);
				
				for (auto& Pair : SubRequirements)
				{
					ProcessClass(Pair.Key, Pair.Value);
				}
			}
		}
	}

	// 수집된 요구사항대로 풀 생성 및 큐 프리로딩
	for (auto& Pair : TotalRequirements)
	{
		PrewarmPool(Pair.Key, Pair.Value);
	}
	PrewarmGameplayCues(TotalCueTags);
}

void UBrawlPoolSubsystem::PrewarmGameplayCues(const FGameplayTagContainer& CueTags)
{
	UWorld* World = GetWorld();
	if (!World || CueTags.IsEmpty()) return;

	UGameplayCueManager* GCM = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	if (!GCM) return;

	// GameplayCue 실행 시 타겟 액터가 필요하므로 WorldSettings를 더미 타겟으로 사용합니다.
	AActor* TargetActor = Cast<AActor>(World->GetWorldSettings());
	if (!TargetActor) return;

	for (auto It = CueTags.CreateConstIterator(); It; ++It)
	{
		FGameplayTag Tag = *It;
		
		if (PreloadedTags.Contains(Tag)) continue;

		// 매니저가 해당 태그의 Notify 객체를 로드하고 캐싱하도록 유도합니다.
		GCM->HandleGameplayCue(TargetActor, Tag, EGameplayCueEvent::WhileActive, FGameplayCueParameters());
		
		PreloadedTags.Add(Tag);
		UE_LOG(LogTemp, Log, TEXT("PoolSubsystem: Pre-loading GameplayCue Tag [%s]"), *Tag.ToString());
	}
}
