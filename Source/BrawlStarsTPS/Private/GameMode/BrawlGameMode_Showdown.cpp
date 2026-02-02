// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BrawlGameMode_Showdown.h"
#include "Environment/BrawlPowerCubeBox.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "BrawlCharacter.h"
#include "AIController.h"

ABrawlGameMode_Showdown::ABrawlGameMode_Showdown()
{
	// 기본 설정
	MaxPowerCubeBoxes = 15;
}

void ABrawlGameMode_Showdown::BeginPlay()
{
	Super::BeginPlay();

	// 상자 스폰
	SpawnPowerCubeBoxes();

	// 초기 생존자 수 계산 (플레이어 + AI)
	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);
	AliveBrawlerCount = FoundBrawlers.Num();

	UE_LOG(LogTemp, Log, TEXT("Showdown Mode Started. Alive Brawlers: %d"), AliveBrawlerCount);
}

void ABrawlGameMode_Showdown::NotifyKill(AActor* Killer, AActor* Victim)
{
	Super::NotifyKill(Killer, Victim);

	// 생존자 수 감소
	AliveBrawlerCount--;
	
	UE_LOG(LogTemp, Log, TEXT("Brawler Killed. Alive Brawlers: %d"), AliveBrawlerCount);

	CheckGameEndCondition();
}

void ABrawlGameMode_Showdown::SpawnPowerCubeBoxes()
{
	if (!PowerCubeBoxClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerCubeBoxClass is NOT SET in GameMode!"));
		return;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("NavigationSystem is NOT Valid!"));
		return;
	}

	int32 SpawnedCount = 0;
	int32 MaxAttempts = MaxPowerCubeBoxes * 2; // 실패 가능성 고려하여 시도 횟수 여유 있게 설정

	for (int32 i = 0; i < MaxAttempts; i++)
	{
		if (SpawnedCount >= MaxPowerCubeBoxes) break;

		FNavLocation RandomLocation;
		// 맵 전체 범위에서 랜덤 위치 탐색 (반경은 맵 크기에 맞게 조정 필요, 여기서는 10000.0f로 가정)
		if (NavSystem->GetRandomPointInNavigableRadius(FVector::ZeroVector, 10000.0f, RandomLocation))
		{
			FVector SpawnLocation = RandomLocation.Location;
			SpawnLocation.Z += 50.0f; // 바닥에 파묻히지 않게 약간 띄움

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			ABrawlPowerCubeBox* NewBox = GetWorld()->SpawnActor<ABrawlPowerCubeBox>(PowerCubeBoxClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
			if (NewBox)
			{
				SpawnedCount++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d PowerCube Boxes."), SpawnedCount);
}

void ABrawlGameMode_Showdown::CheckGameEndCondition()
{
	if (AliveBrawlerCount <= 1)
	{
		// 게임 종료
		// 최후의 1인이 플레이어인지 확인
		bool bIsPlayerWinner = false;
		
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		if (PlayerPawn)
		{
			ABrawlCharacter* PlayerCharacter = Cast<ABrawlCharacter>(PlayerPawn);
			if (PlayerCharacter && !PlayerCharacter->IsDead())
			{
				bIsPlayerWinner = true;
			}
		}

		EndGame(bIsPlayerWinner);
	}
}

void ABrawlGameMode_Showdown::EndGame(bool bIsPlayerWinner)
{
	if (bIsPlayerWinner)
	{
		UE_LOG(LogTemp, Log, TEXT("GAME OVER! Player WINS!"));
		// TODO: 승리 UI 표시
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GAME OVER! Player LOST!"));
		// TODO: 패배 UI 표시
	}

	// 게임 일시 정지 등의 처리
}
