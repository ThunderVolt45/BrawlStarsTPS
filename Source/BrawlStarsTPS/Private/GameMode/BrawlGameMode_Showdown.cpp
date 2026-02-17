// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BrawlGameMode_Showdown.h"
#include "Environment/BrawlPowerCubeBox.h"
#include "Environment/BrawlPowerCube.h" 
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "BrawlCharacter.h"
#include "GameMode/BrawlSpawnPoint.h"
#include "Data/BrawlSpawnPointType.h"
#include "AbilitySystemBlueprintLibrary.h" 
#include "BrawlAttributeSet.h" 
#include "BrawlGameState.h"
#include "BrawlPlayerState.h"
#include "BrawlPoolSubsystem.h"

ABrawlGameMode_Showdown::ABrawlGameMode_Showdown()
{
	GameModeType = EBrawlGameModeType::Showdown;
	MaxPowerCubeBoxes = 15;
	MaxBots = 9; // 플레이어 포함 10명
	StartDelay = 5.0f;
	PoisonStartDelay = 5.0f;
}

void ABrawlGameMode_Showdown::BeginPlay()
{
	Super::BeginPlay();

	// 상자 및 봇 스폰
	SpawnPowerCubeBoxes();
	SpawnBots();
	
	// 브롤러 처치 시 드롭될 큐브들을 위해 넉넉하게 추가 프리워밍 (상자 요구량 외 여분 20개)
	if (UBrawlPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UBrawlPoolSubsystem>())
	{
		if (PowerCubeClass)
		{
			PoolSubsystem->PrewarmPool(PowerCubeClass, 20);
		}
	}

	// 플레이어 팀 설정
	SetupTeams();

	// 초기 생존자 수 계산 (플레이어 + AI)
	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);
	
	AliveBrawlerCount = 0;
	for (AActor* Actor : FoundBrawlers)
	{
		if (IsActiveHero(Actor))
		{
			AliveBrawlerCount++;
		}
	}

	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		GS->SetAliveBrawlerCount(AliveBrawlerCount);
	}
}

void ABrawlGameMode_Showdown::SetupTeams()
{
	// 쇼다운은 모두가 적 (255 할당)
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ABrawlPlayerState* PS = PC->GetPlayerState<ABrawlPlayerState>())
		{
			PS->SetTeamID(255);
			AssignedTeams.Add(PC, 255);
			
			if (ABrawlCharacter* Char = Cast<ABrawlCharacter>(PC->GetPawn()))
			{
				Char->SetGenericTeamId(FGenericTeamId(255));
			}
		}
	}
}

void ABrawlGameMode_Showdown::SpawnBots()
{
	if (AICharacterClasses.Num() == 0 || MaxBots <= 0) return;

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);
	
	for (int32 i = 0; i < FoundSpawnPoints.Num(); i++)
	{
		int32 RandIndex = FMath::RandRange(i, FoundSpawnPoints.Num() - 1);
		FoundSpawnPoints.Swap(i, RandIndex);
	}

	int32 SpawnedCount = 0;
	for (AActor* Actor : FoundSpawnPoints)
	{
		if (SpawnedCount >= MaxBots) break;

		if (ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SP->SpawnPointType == EBrawlSpawnPointType::Brawler)
			{
				// 모두 255 팀으로 설정 (서로 적)
				if (SpawnBotAt(255, SP))
				{
					SpawnedCount++;
				}
			}
		}
	}
}

void ABrawlGameMode_Showdown::StartMatch()
{
	if (bHasMatchStarted) return;
	
	Super::StartMatch(); // MatchStart -> Playing 전이 포함

	// 독구름 로직은 Playing 상태 진입 시점에 맞춰 StartMatch(부모) 내부 혹은 여기서 직접 시작
	// 부모 StartMatch는 1.5초 뒤에 Playing으로 바꾸므로, 여기서 바로 시작하거나 타이머 사용
	StartPoisonLogic();
}

void ABrawlGameMode_Showdown::NotifyKill(AActor* Killer, AActor* Victim)
{
	Super::NotifyKill(Killer, Victim);

	if (IsHero(Victim))
	{
		DropPowerCubes(Victim);
		AliveBrawlerCount--;
		
		if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
		{
			GS->SetAliveBrawlerCount(AliveBrawlerCount);
		}
		
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
	UBrawlPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UBrawlPoolSubsystem>();

	for (int32 i = 0; i < DropCount; i++)
	{
		FVector2D RandomOffset = FMath::RandPointInCircle(100.0f);
		FVector SpawnLocation = CenterLocation + FVector(RandomOffset.X, RandomOffset.Y, 0);
		SpawnLocation.Z = FMath::Max(SpawnLocation.Z, 10.0f);
		FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

		if (PoolSubsystem)
		{
			PoolSubsystem->GetFromPool(PowerCubeClass, SpawnTransform);
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			GetWorld()->SpawnActor<ABrawlPowerCube>(PowerCubeClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		}
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
		if (ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SP->SpawnPointType == EBrawlSpawnPointType::PowerCubeBox)
			{
				if (SpawnedCount >= MaxPowerCubeBoxes) break;
				if (SP->IsOccupied(50.0f)) continue;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				if (GetWorld()->SpawnActor<AActor>(PowerCubeBoxClass, SP->GetActorLocation(), SP->GetActorRotation(), SpawnParams))
				{
					SpawnedCount++;
				}
			}
		}
	}

	// 부족한 경우 랜덤 위치 스폰 (NavMesh 기반)
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

					// 주변에 다른 상자가 있는지 확인 (IsOccupied 대용)
					TArray<AActor*> Overlapping;
					TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
					ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
					
					if (!UKismetSystemLibrary::SphereOverlapActors(GetWorld(), SpawnLocation, 100.0f, ObjectTypes, ABrawlPowerCubeBox::StaticClass(), TArray<AActor*>(), Overlapping))
					{
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
