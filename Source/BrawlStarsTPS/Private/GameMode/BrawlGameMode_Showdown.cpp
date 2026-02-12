// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BrawlGameMode_Showdown.h"
#include "Environment/BrawlPowerCubeBox.h"
#include "Environment/BrawlPowerCube.h" 
#include "Environment/BrawlPoisonZone.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "BrawlCharacter.h"
#include "GameMode/BrawlSpawnPoint.h"
#include "Data/BrawlSpawnPointType.h"
#include "AbilitySystemBlueprintLibrary.h" 
#include "BrawlAttributeSet.h" 
#include "BrawlGameState.h"
#include "BrawlPlayerState.h"
#include "BrawlStarsTPSPlayerController.h"
#include "AI/BrawlAIController.h"

ABrawlGameMode_Showdown::ABrawlGameMode_Showdown()
{
	MaxPowerCubeBoxes = 15;
	MaxBots = 5;
	StartDelay = 5.0f;
	PoisonStartDelay = 5.0f;
}

void ABrawlGameMode_Showdown::BeginPlay()
{
	Super::BeginPlay();

	// 상자 및 봇 스폰
	SpawnPowerCubeBoxes();
	SpawnBots();

	// 초기 생존자 수 계산 (플레이어 + AI) - 상자는 제외
	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);
	
	AliveBrawlerCount = 0;
	for (AActor* Actor : FoundBrawlers)
	{
		if (Actor && !Actor->IsA<ABrawlPowerCubeBox>())
		{
			AliveBrawlerCount++;
		}
	}

	// GameState 동기화
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		GS->SetAliveBrawlerCount(AliveBrawlerCount);
	}
}

void ABrawlGameMode_Showdown::StartMatch()
{
	if (bHasMatchStarted) return;
	
	Super::StartMatch();

	// 독구름 로직 시작
	StartPoisonLogic();

	UE_LOG(LogTemp, Log, TEXT("Showdown Match Officially Started!"));
}

void ABrawlGameMode_Showdown::NotifyKill(AActor* Killer, AActor* Victim)
{
	Super::NotifyKill(Killer, Victim);

	// 상자가 아닌 경우에만 파워 큐브 추가 드랍 및 생존 카운트 감소
	if (Victim && !Victim->IsA<ABrawlPowerCubeBox>())
	{
		// 브롤러가 죽었을 때만 큐브 드랍 (상자는 본인의 Die()에서 드랍)
		DropPowerCubes(Victim);

		// 생존자 수 감소
		AliveBrawlerCount--;
		
		// GameState 동기화
		if (ABrawlGameState* BrawlGameState = GetGameState<ABrawlGameState>())
		{
			BrawlGameState->SetAliveBrawlerCount(AliveBrawlerCount);
		}
		
		UE_LOG(LogTemp, Log, TEXT("Brawler Killed. Alive Brawlers: %d"), AliveBrawlerCount);

		CheckGameEndCondition();
	}
}

void ABrawlGameMode_Showdown::DropPowerCubes(AActor* Victim)
{
	if (!Victim || !PowerCubeClass) return;

	int32 DropCount = 1;

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Victim))
	{
		bool bFound = false;
		float CurrentCubes = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetPowerCubeCountAttribute(), bFound);
		
		if (bFound && CurrentCubes > 0.0f)
		{
			DropCount += FMath::FloorToInt(CurrentCubes / 2.0f);
		}
	}

	FVector CenterLocation = Victim->GetActorLocation();
	CenterLocation.Z -= 40.0f;

	for (int32 i = 0; i < DropCount; i++)
	{
		FVector2D RandomOffset = FMath::RandPointInCircle(100.0f);
		FVector SpawnLocation = CenterLocation + FVector(RandomOffset.X, RandomOffset.Y, 0);
		SpawnLocation.Z = FMath::Max(SpawnLocation.Z, 10.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<ABrawlPowerCube>(PowerCubeClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	}
}

void ABrawlGameMode_Showdown::SpawnPowerCubeBoxes()
{
	if (!PowerCubeBoxClass) return;

	int32 SpawnedCount = 0;
	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);

	for (int32 i = 0; i < FoundSpawnPoints.Num(); i++)
	{
		int32 RandIndex = FMath::RandRange(i, FoundSpawnPoints.Num() - 1);
		FoundSpawnPoints.Swap(i, RandIndex);
	}

	for (AActor* Actor : FoundSpawnPoints)
	{
		if (ABrawlSpawnPoint* SpawnPoint = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SpawnPoint->SpawnPointType == EBrawlSpawnPointType::PowerCubeBox)
			{
				if (SpawnedCount >= MaxPowerCubeBoxes) break;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				if (GetWorld()->SpawnActor<AActor>(PowerCubeBoxClass, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation(), SpawnParams))
				{
					SpawnedCount++;
				}
			}
		}
	}

	if (SpawnedCount < MaxPowerCubeBoxes)
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSystem)
		{
			int32 RemainingToSpawn = MaxPowerCubeBoxes - SpawnedCount;
			for (int32 i = 0; i < RemainingToSpawn * 3; i++)
			{
				if (SpawnedCount >= MaxPowerCubeBoxes) break;

				FNavLocation RandomLocation;
				if (NavSystem->GetRandomPointInNavigableRadius(FVector::ZeroVector, 10000.0f, RandomLocation))
				{
					FVector SpawnLocation = RandomLocation.Location;
					SpawnLocation.Z += 50.0f;

					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					if (GetWorld()->SpawnActor<AActor>(PowerCubeBoxClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
					{
						SpawnedCount++;
					}
				}
			}
		}
	}
}

void ABrawlGameMode_Showdown::CheckGameEndCondition()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	ABrawlCharacter* PlayerCharacter = Cast<ABrawlCharacter>(PlayerPawn);
	
	if (PlayerCharacter && PlayerCharacter->IsDead())
	{
		EndGame(false, AliveBrawlerCount + 1);
		return;
	}

	if (AliveBrawlerCount <= 1)
	{
		if (PlayerCharacter && !PlayerCharacter->IsDead())
		{
			EndGame(true, 1);
		}
		else
		{
			EndGame(false, 1);
		}
	}
}