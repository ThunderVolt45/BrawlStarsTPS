#include "GameMode/BrawlGameMode_Knockout.h"
#include "BrawlCharacter.h"
#include "BrawlPlayerState.h"
#include "BrawlGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/BrawlSpawnPoint.h"
#include "AI/BrawlAIController.h"
#include "Environment/BrawlPoisonZone.h"

ABrawlGameMode_Knockout::ABrawlGameMode_Knockout()
{
	MaxBots = 5; // 플레이어 포함 6명을 만들기 위해 5명의 봇 추가
	StartDelay = 5.0f;
	PoisonStartDelay = 20.0f; // 녹아웃은 독구름이 조금 늦게 시작됨
}

void ABrawlGameMode_Knockout::BeginPlay()
{
	Super::BeginPlay();

	// 봇 스폰 (3v3 설정)
	SpawnBots();

	// 첫 라운드 준비
	CurrentRound = 1;
}

void ABrawlGameMode_Knockout::StartMatch()
{
	if (bHasMatchStarted) return;
	
	Super::StartMatch();

	// 독구름 로직 시작
	StartPoisonLogic();

	// 생존자 수 초기화
	Team1AliveCount = 0;
	Team2AliveCount = 0;

	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);

	for (AActor* Actor : FoundBrawlers)
	{
		if (ABrawlCharacter* Brawler = Cast<ABrawlCharacter>(Actor))
		{
			if (Brawler->GetTeamID() == 0 || Brawler->GetTeamID() == 1)
			{
				if (Brawler->GetTeamID() == 0) Team1AliveCount++;
				else Team2AliveCount++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Knockout Round %d Started! Team1: %d, Team2: %d"), CurrentRound, Team1AliveCount, Team2AliveCount);
}

void ABrawlGameMode_Knockout::NotifyKill(AActor* Killer, AActor* Victim)
{
	Super::NotifyKill(Killer, Victim);

	if (ABrawlCharacter* VictimBrawler = Cast<ABrawlCharacter>(Victim))
	{
		if (VictimBrawler->GetTeamID() == 0)
		{
			Team1AliveCount--;
		}
		else if (VictimBrawler->GetTeamID() == 1)
		{
			Team2AliveCount--;
		}

		UE_LOG(LogTemp, Log, TEXT("Knockout: Brawler Killed. Team1 Alive: %d, Team2 Alive: %d"), Team1AliveCount, Team2AliveCount);

		CheckRoundEndCondition();
	}
}

void ABrawlGameMode_Knockout::CheckRoundEndCondition()
{
	if (Team1AliveCount <= 0)
	{
		EndRound(1); // Team 2 wins
	}
	else if (Team2AliveCount <= 0)
	{
		EndRound(0); // Team 1 wins
	}
}

void ABrawlGameMode_Knockout::EndRound(int32 WinningTeam)
{
	// 독구름 중지
	GetWorld()->GetTimerManager().ClearTimer(PoisonUpdateTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(PoisonDamageTimerHandle);
	if (PoisonZoneInstance)
	{
		PoisonZoneInstance->Destroy();
		PoisonZoneInstance = nullptr;
	}

	if (WinningTeam == 0) Team1Wins++;
	else Team2Wins++;

	UE_LOG(LogTemp, Log, TEXT("Round %d Ended. Winner: Team %d. Score: %d - %d"), CurrentRound, WinningTeam, Team1Wins, Team2Wins);

	if (Team1Wins >= RequiredWins || Team2Wins >= RequiredWins)
	{
		// 게임 종료
		bool bIsPlayerWinner = false;
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (ABrawlCharacter* PlayerChar = Cast<ABrawlCharacter>(PlayerPawn))
			{
				if (PlayerChar->GetTeamID() == WinningTeam) bIsPlayerWinner = true;
			}
		}
		EndGame(bIsPlayerWinner, WinningTeam);
	}
	else
	{
		// 다음 라운드 준비
		CurrentRound++;
		bHasMatchStarted = false;

		if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
		{
			GS->SetMatchState(EBrawlMatchState::Intro);
		}

		FTimerHandle NextRoundTimerHandle;
		GetWorldTimerManager().SetTimer(NextRoundTimerHandle, this, &ABrawlGameMode_Knockout::StartNewRound, RoundResetDelay, false);
	}
}

void ABrawlGameMode_Knockout::StartNewRound()
{
	ResetBrawlersForRound();
	
	// StartMatch 호출 (Intro 시간 고려)
	FTimerHandle StartTimerHandle;
	GetWorldTimerManager().SetTimer(StartTimerHandle, this, &ABrawlGameMode_Knockout::StartMatch, StartDelay, false);
}

void ABrawlGameMode_Knockout::ResetBrawlersForRound()
{
	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);

	for (AActor* Actor : FoundBrawlers)
	{
		if (ABrawlCharacter* Brawler = Cast<ABrawlCharacter>(Actor))
		{
			// 팀에 맞는 스폰 포인트 찾기
			AController* Controller = Brawler->GetController();
			AActor* SpawnPoint = ChoosePlayerStart(Controller);
			
			if (SpawnPoint)
			{
				Brawler->RespawnAt(SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation());
			}
		}
	}
}

void ABrawlGameMode_Knockout::SpawnBots()
{
	// 3v3 설정을 위해 팀 할당 로직 오버라이드
	if (AICharacterClasses.Num() == 0 || MaxBots <= 0) return;

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);
	
	// 셔플
	for (int32 i = 0; i < FoundSpawnPoints.Num(); i++)
	{
		int32 RandIndex = FMath::RandRange(i, FoundSpawnPoints.Num() - 1);
		FoundSpawnPoints.Swap(i, RandIndex);
	}

	// 플레이어 팀 ID 확인
	int32 PlayerTeamID = 0;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (ABrawlCharacter* PlayerChar = Cast<ABrawlCharacter>(PlayerPawn))
		{
			// 플레이어에게 팀 ID 0 할당 (보통 그렇다고 가정)
			if (ABrawlPlayerState* PS = PlayerChar->GetPlayerState<ABrawlPlayerState>())
			{
				PS->SetTeamID(0);
				PlayerTeamID = 0;
			}
		}
	}

	int32 Team0Count = 1; // 플레이어 포함
	int32 Team1Count = 0;
	int32 MaxTeamSize = 3;

	int32 SpawnedCount = 0;
	for (AActor* Actor : FoundSpawnPoints)
	{
		if (SpawnedCount >= MaxBots) break;

		if (ABrawlSpawnPoint* SpawnPoint = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SpawnPoint->SpawnPointType == EBrawlSpawnPointType::Brawler)
			{
				// 스폰 포인트의 팀 ID에 맞춰서 스폰
				int32 TargetTeam = SpawnPoint->TeamID;
				if (TargetTeam == 255) continue; // 공용 포인트는 무시 (Knockout은 팀 포인트 필수)

				if (TargetTeam == 0 && Team0Count >= MaxTeamSize) continue;
				if (TargetTeam == 1 && Team1Count >= MaxTeamSize) continue;

				TSubclassOf<ABrawlCharacter> BotClass = AICharacterClasses[FMath::RandRange(0, AICharacterClasses.Num() - 1)];
				if (BotClass)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
					
					ABrawlCharacter* NewBot = GetWorld()->SpawnActor<ABrawlCharacter>(BotClass, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation(), SpawnParams);
					if (NewBot)
					{
						NewBot->SpawnDefaultController();
						ConfigureAI(NewBot->GetController(), TargetTeam);
						
						if (TargetTeam == 0) Team0Count++;
						else Team1Count++;
						
						SpawnedCount++;
					}
				}
			}
		}
	}
}