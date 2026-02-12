#include "GameMode/BrawlGameMode_Knockout.h"
#include "BrawlCharacter.h"
#include "BrawlPlayerState.h"
#include "BrawlGameState_Knockout.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/BrawlSpawnPoint.h"
#include "Environment/BrawlPoisonZone.h"
#include "GameFramework/PlayerInput.h"

ABrawlGameMode_Knockout::ABrawlGameMode_Knockout()
{
	GameStateClass = ABrawlGameState_Knockout::StaticClass();
	MaxBots = 5; // 플레이어 포함 6명을 만들기 위해 5명의 봇 추가
	StartDelay = 5.0f;
	PoisonStartDelay = 20.0f;
}

void ABrawlGameMode_Knockout::BeginPlay()
{
	Super::BeginPlay();

	// 봇 스폰 (3v3 설정)
	SpawnBots();
	
	// 플레이어 팀 설정 및 초기 위치 보정
	SetupTeams();

	// 첫 라운드 준비
	CurrentRound = 1;
}

void ABrawlGameMode_Knockout::PostLogin(APlayerController* NewPlayer)
{
	// 1. Super::PostLogin 호출 전에 맵에 먼저 등록
	if (NewPlayer)
	{
		AssignedTeams.Add(NewPlayer, 0);
	}

	Super::PostLogin(NewPlayer);

	// 2. PlayerState에 팀 설정
	if (ABrawlPlayerState* PS = NewPlayer->GetPlayerState<ABrawlPlayerState>())
	{
		PS->SetTeamID(0);
	}
}

void ABrawlGameMode_Knockout::SetupTeams()
{
	// Knockout 전용 팀 할당 (3v3) 수행
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (ABrawlPlayerState* PS = PC->GetPlayerState<ABrawlPlayerState>())
			{
				// 플레이어는 무조건 팀 0
				PS->SetTeamID(0);
				AssignedTeams.Add(PC, 0);
				
				APawn* P = PC->GetPawn();
				bool bNeedsRestart = false;

				if (!P)
				{
					bNeedsRestart = true;
				}
				else
				{
					// 위치 보정이 필요한 경우
					if (P->GetActorLocation().Z < -100.0f || P->GetActorLocation().IsNearlyZero(1.0f))
					{
						bNeedsRestart = true;
					}
					else
					{
						// 현재 위치 주변의 스폰 포인트를 찾아 팀 확인
						TArray<AActor*> NearbySpawnPoints;
						TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
						ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
						ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
						
						TArray<AActor*> OutActors;
						UKismetSystemLibrary::SphereOverlapActors(GetWorld(), P->GetActorLocation(), 200.0f, ObjectTypes, ABrawlSpawnPoint::StaticClass(), TArray<AActor*>(), OutActors);
						
						for (AActor* Actor : OutActors)
						{
							if (ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor))
							{
								if (SP->SpawnPointType == EBrawlSpawnPointType::Brawler && SP->TeamID != 255 && SP->TeamID != 0)
								{
									// 적 진영 스폰 지점 근처에 있다면 재배치 필요
									bNeedsRestart = true;
									UE_LOG(LogTemp, Warning, TEXT("SetupTeams: Player [%s] spawned at WRONG Team SpawnPoint [%d]. Restarting..."), *PC->GetName(), SP->TeamID);
									break;
								}
							}
						}
					}
				}

				if (bNeedsRestart)
				{
					RestartPlayer(PC);
					P = PC->GetPawn();
				}

				if (P)
				{
					if (ABrawlCharacter* Char = Cast<ABrawlCharacter>(P))
					{
						Char->SetGenericTeamId(FGenericTeamId(0));
					}
				}
			}
		}
	}
}

void ABrawlGameMode_Knockout::StartMatch()
{
	if (bHasMatchStarted) return;
	bHasMatchStarted = true;
	
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		GS->SetMatchState(EBrawlMatchState::MatchStart);

		FTimerHandle PlayingStateTimerHandle;
		GetWorldTimerManager().SetTimer(PlayingStateTimerHandle, [this, GS]()
		{
			if (GS)
			{
				GS->SetMatchState(EBrawlMatchState::Playing);
				StartPoisonLogic();
			}
		}, 1.5f, false);
	}

	// 생존자 수 초기화
	Team1AliveCount = 0;
	Team2AliveCount = 0;

	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);

	for (AActor* Actor : FoundBrawlers)
	{
		if (ABrawlCharacter* Brawler = Cast<ABrawlCharacter>(Actor))
		{
			if (Brawler->GetTeamID() == 0) Team1AliveCount++;
			else if (Brawler->GetTeamID() == 1) Team2AliveCount++;
		}
	}
}

void ABrawlGameMode_Knockout::NotifyKill(AActor* Killer, AActor* Victim)
{
	Super::NotifyKill(Killer, Victim);

	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		if (GS->GetMatchState() != EBrawlMatchState::Playing) return;
	}

	if (ABrawlCharacter* VictimBrawler = Cast<ABrawlCharacter>(Victim))
	{
		if (VictimBrawler->GetTeamID() == 0) Team1AliveCount--;
		else if (VictimBrawler->GetTeamID() == 1) Team2AliveCount--;

		CheckRoundEndCondition();
	}
}

void ABrawlGameMode_Knockout::CheckRoundEndCondition()
{
	if (Team1AliveCount <= 0) EndRound(1);
	else if (Team2AliveCount <= 0) EndRound(0);
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

	if (ABrawlGameState_Knockout* KGS = GetGameState<ABrawlGameState_Knockout>())
	{
		KGS->SetTeamWins(Team1Wins, Team2Wins);
		KGS->SetLastRoundWinner(WinningTeam);
		KGS->AddRoundWinner(WinningTeam);
	}

	if (Team1Wins >= RequiredWins || Team2Wins >= RequiredWins)
	{
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
		CurrentRound++;
		bHasMatchStarted = false;

		if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
		{
			GS->SetMatchState(EBrawlMatchState::Intermission);
		}

		FTimerHandle NextRoundTimerHandle;
		GetWorldTimerManager().SetTimer(NextRoundTimerHandle, this, &ABrawlGameMode_Knockout::StartNewRound, RoundResetDelay, false);
	}
}

void ABrawlGameMode_Knockout::StartNewRound()
{
	ResetBrawlersForRound();
	
	Team1AliveCount = 0;
	Team2AliveCount = 0;

	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);

	for (AActor* Actor : FoundBrawlers)
	{
		if (ABrawlCharacter* Brawler = Cast<ABrawlCharacter>(Actor))
		{
			if (Brawler->GetTeamID() == 0) Team1AliveCount++;
			else if (Brawler->GetTeamID() == 1) Team2AliveCount++;
		}
	}

	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		bHasMatchStarted = true;
		GS->SetMatchState(EBrawlMatchState::Playing);
		StartPoisonLogic();
	}
}

void ABrawlGameMode_Knockout::ResetBrawlersForRound()
{
	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);

	for (AActor* Actor : FoundBrawlers)
	{
		if (ABrawlCharacter* Brawler = Cast<ABrawlCharacter>(Actor))
		{
			AController* Controller = Brawler->GetController();
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				if (PC->PlayerInput) PC->PlayerInput->FlushPressedKeys();
			}

			AActor* SpawnPoint = FindPlayerStart(Controller);
			if (SpawnPoint)
			{
				Brawler->RespawnAt(SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation());
			}
		}
	}
}

void ABrawlGameMode_Knockout::SpawnBots()
{
	if (AICharacterClasses.Num() == 0) return;

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);

	int32 Team0BotsToSpawn = 2;
	int32 Team1BotsToSpawn = 3;

	for (AActor* Actor : FoundSpawnPoints)
	{
		ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor);
		if (!SP || SP->SpawnPointType != EBrawlSpawnPointType::Brawler) continue;

		if (SP->TeamID == 0 && Team0BotsToSpawn > 0)
		{
			if (SpawnBotAt(0, SP)) Team0BotsToSpawn--;
		}
		else if (SP->TeamID == 1 && Team1BotsToSpawn > 0)
		{
			if (SpawnBotAt(1, SP)) Team1BotsToSpawn--;
		}
	}
}
