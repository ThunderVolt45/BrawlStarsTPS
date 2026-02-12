#include "GameMode/BrawlGameMode_Knockout.h"
#include "BrawlCharacter.h"
#include "BrawlPlayerState.h"
#include "BrawlGameState_Knockout.h"
#include "BrawlStarsTPSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/BrawlSpawnPoint.h"
#include "AI/BrawlAIController.h"
#include "Environment/BrawlPoisonZone.h"
#include "GameFramework/PlayerInput.h"

ABrawlGameMode_Knockout::ABrawlGameMode_Knockout()
{
	GameStateClass = ABrawlGameState_Knockout::StaticClass();
	MaxBots = 5; // 플레이어 포함 6명을 만들기 위해 5명의 봇 추가
	StartDelay = 5.0f;
	PoisonStartDelay = 20.0f; // 녹아웃은 독구름이 조금 늦게 시작됨
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
	Super::PostLogin(NewPlayer);

	if (ABrawlPlayerState* PS = NewPlayer->GetPlayerState<ABrawlPlayerState>())
	{
		// 플레이어 팀 할당
		if (PS->GetTeamID() == 255) PS->SetTeamID(0);
		AssignedTeams.Add(NewPlayer, PS->GetTeamID());
		
		UE_LOG(LogTemp, Log, TEXT("Knockout: PostLogin - Player [%s] TeamID: %d"), *NewPlayer->GetName(), PS->GetTeamID());
	}
}

void ABrawlGameMode_Knockout::SetupTeams()
{
	// 한 프레임 뒤에 실행하여 월드 로드 및 초기 스폰 완료를 기다림
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				if (ABrawlPlayerState* PS = PC->GetPlayerState<ABrawlPlayerState>())
				{
					// 플레이어는 무조건 팀 0
					PS->SetTeamID(0);
					AssignedTeams.Add(PC, 0);
					
					if (APawn* P = PC->GetPawn())
					{
						// (0,0,0)에 있거나 너무 낮게 스폰된 경우 재배치 (Bounty 로직)
						if (P->GetActorLocation().Z < -100.0f || P->GetActorLocation().IsNearlyZero(1.0f))
						{
							UE_LOG(LogTemp, Warning, TEXT("Knockout: Player [%s] is at invalid location %s. Restarting..."), 
								*P->GetName(), *P->GetActorLocation().ToString());
							RestartPlayer(PC);
						}
						
						if (ABrawlCharacter* Char = Cast<ABrawlCharacter>(PC->GetPawn()))
						{
							Char->SetGenericTeamId(FGenericTeamId(0));
						}
					}
					else
					{
						RestartPlayer(PC);
					}
				}
			}
		}
	});
}

void ABrawlGameMode_Knockout::StartMatch()
{
	if (bHasMatchStarted) return;
	bHasMatchStarted = true;
	
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		// 1. MatchStart 상태로 전이 (START 연출)
		GS->SetMatchState(EBrawlMatchState::MatchStart);

		// 2. 연출 시간 뒤 Playing 상태로 전이
		FTimerHandle PlayingStateTimerHandle;
		GetWorldTimerManager().SetTimer(PlayingStateTimerHandle, [this, GS]()
		{
			if (GS)
			{
				GS->SetMatchState(EBrawlMatchState::Playing);
				// 첫 라운드도 Playing 상태 진입 시 독구름 시작
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
			if (Brawler->GetTeamID() == 0 || Brawler->GetTeamID() == 1)
			{
				if (Brawler->GetTeamID() == 0) Team1AliveCount++;
				else Team2AliveCount++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Knockout Round %d Started! Team1: %d, Team2: %d"), CurrentRound, Team1AliveCount, Team2AliveCount);
}

UClass* ABrawlGameMode_Knockout::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (!InController) return nullptr;

	// 1. 할당된 AI 클래스가 있다면 반환
	if (AssignedAIClasses.Contains(InController))
	{
		return AssignedAIClasses[InController];
	}

	// 2. AI 컨트롤러인데 할당된 클래스가 없는 경우 랜덤하게 선택 (Bounty 스타일 안전장치)
	if (InController->IsA<AAIController>() && AICharacterClasses.Num() > 0)
	{
		int32 CharIndex = FMath::RandRange(0, AICharacterClasses.Num() - 1);
		TSubclassOf<ABrawlCharacter> RandomClass = AICharacterClasses[CharIndex];
		
		// 나중을 위해 저장
		AssignedAIClasses.Add(InController, RandomClass);
		
		UE_LOG(LogTemp, Log, TEXT("Knockout: Assigned random class [%s] to AI Controller [%s]"), 
			*RandomClass->GetName(), *InController->GetName());
			
		return RandomClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* ABrawlGameMode_Knockout::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!Player) return nullptr;

	int32 TeamID = -1;

	// 1. AssignedTeams 맵에서 먼저 확인
	if (AssignedTeams.Contains(Player))
	{
		TeamID = AssignedTeams[Player];
	}
	// 2. PlayerState에서 확인
	else if (ABrawlPlayerState* PS = Player->GetPlayerState<ABrawlPlayerState>())
	{
		TeamID = PS->GetTeamID();
	}

	// 3. 플레이어 컨트롤러인데 팀이 안 정해졌다면 기본값 0
	if ((TeamID == -1 || TeamID == 255) && Player->IsPlayerController())
	{
		TeamID = 0;
	}

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);

	TArray<ABrawlSpawnPoint*> TeamSpawnPoints;
	TArray<ABrawlSpawnPoint*> ValidSpawnPoints;

	for (AActor* Actor : FoundSpawnPoints)
	{
		ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor);
		if (SP && SP->SpawnPointType == EBrawlSpawnPointType::Brawler && (SP->TeamID == TeamID || SP->TeamID == 255))
		{
			TeamSpawnPoints.Add(SP);

			// 점유 상태 확인 (캐릭터가 이미 있는지)
			TArray<AActor*> OverlappingActors;
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
			
			if (!UKismetSystemLibrary::SphereOverlapActors(GetWorld(), SP->GetActorLocation(), 100.0f, ObjectTypes, ABrawlCharacter::StaticClass(), TArray<AActor*>(), OverlappingActors))
			{
				ValidSpawnPoints.Add(SP);
			}
		}
	}

	AActor* SelectedSpot = nullptr;
	if (ValidSpawnPoints.Num() > 0)
	{
		SelectedSpot = ValidSpawnPoints[FMath::RandRange(0, ValidSpawnPoints.Num() - 1)];
	}
	else if (TeamSpawnPoints.Num() > 0)
	{
		SelectedSpot = TeamSpawnPoints[FMath::RandRange(0, TeamSpawnPoints.Num() - 1)];
	}

	if (SelectedSpot)
	{
		UE_LOG(LogTemp, Log, TEXT("Knockout: Found SpawnPoint [%s] for Team %d"), *SelectedSpot->GetName(), TeamID);
		return SelectedSpot;
	}

	UE_LOG(LogTemp, Warning, TEXT("Knockout: Failed to find Team %d SpawnPoint, falling back to Super"), TeamID);
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ABrawlGameMode_Knockout::NotifyKill(AActor* Killer, AActor* Victim)
{
	Super::NotifyKill(Killer, Victim);

	// 매치 진행 중(Playing)일 때만 처치 및 라운드 종료 로직 수행
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		if (GS->GetMatchState() != EBrawlMatchState::Playing) return;
	}

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

	// GameState_Knockout 정보 업데이트
	if (ABrawlGameState_Knockout* KGS = GetGameState<ABrawlGameState_Knockout>())
	{
		KGS->SetTeamWins(Team1Wins, Team2Wins);
		KGS->SetLastRoundWinner(WinningTeam);
	}

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
			GS->SetMatchState(EBrawlMatchState::Intermission);
		}

		FTimerHandle NextRoundTimerHandle;
		GetWorldTimerManager().SetTimer(NextRoundTimerHandle, this, &ABrawlGameMode_Knockout::StartNewRound, RoundResetDelay, false);
	}
}

void ABrawlGameMode_Knockout::StartNewRound()
{
	ResetBrawlersForRound();
	
	// 생존자 수 다시 계산 및 초기화
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

	UE_LOG(LogTemp, Log, TEXT("Knockout Round %d Prepared! Team1 Alive: %d, Team2 Alive: %d"), CurrentRound, Team1AliveCount, Team2AliveCount);

	// 대기 없이 즉시 Playing 상태로 전이
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		bHasMatchStarted = true;
		GS->SetMatchState(EBrawlMatchState::Playing);
				
		// 독구름 다시 시작
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
			// 1. 컨트롤러 입력 상태 강제 초기화 (키 눌림 상태 해제)
			AController* Controller = Brawler->GetController();
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				if (PC->PlayerInput)
				{
					PC->PlayerInput->FlushPressedKeys();
				}
			}

			// 2. 팀에 맞는 스폰 포인트 찾기
			AActor* SpawnPoint = FindPlayerStart(Controller);
			
			if (SpawnPoint)
			{
				// 3. 스폰 포인트의 위치로 즉시 리스폰 (오프셋 없이)
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

	auto SpawnBotForTeam = [&](int32 TeamID, ABrawlSpawnPoint* SP)
	{
		// 스폰 지점 점유 확인
		TArray<AActor*> OverlappingActors;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), SP->GetActorLocation(), 
			50.0f, ObjectTypes, ABrawlCharacter::StaticClass(), TArray<AActor*>(), OverlappingActors))
		{
			return false;
		}

		int32 CharIndex = FMath::RandRange(0, AICharacterClasses.Num() - 1);
		TSubclassOf<ABrawlCharacter> BotClass = AICharacterClasses[CharIndex];

		FActorSpawnParameters AICSpawnParams;
		AICSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		UClass* AICClass = ABrawlAIController::StaticClass();
		if (BotClass.GetDefaultObject()->AIControllerClass)
		{
			AICClass = BotClass.GetDefaultObject()->AIControllerClass;
		}

		ABrawlAIController* AIC = GetWorld()->SpawnActor<ABrawlAIController>(AICClass, SP->GetActorLocation(), SP->GetActorRotation(), AICSpawnParams);
		
		if (AIC)
		{
			AIC->InitPlayerState();

			AssignedAIClasses.Add(AIC, BotClass);
			AssignedTeams.Add(AIC, TeamID);
			
			if (ABrawlPlayerState* PS = AIC->GetPlayerState<ABrawlPlayerState>())
			{
				PS->SetTeamID(TeamID);
			}

			RestartPlayerAtPlayerStart(AIC, SP);

			if (APawn* NewPawn = AIC->GetPawn())
			{
				FVector SpawnLocation = SP->GetActorLocation();
				NewPawn->SetActorLocationAndRotation(SpawnLocation, SP->GetActorRotation());

				if (ABrawlCharacter* NewBot = Cast<ABrawlCharacter>(NewPawn))
				{
					NewBot->SetGenericTeamId(FGenericTeamId(TeamID));
					
					if (GameModeAITree)
					{
						AIC->InjectGameModeSubtree(GameModeAITree);
					}
					// Knockout은 매치 시작 전 대기하므로 AI 활성화는 StartMatch나 NotifyMatchStarted에서 수행하거나
					// 필요에 따라 여기서 설정 (Bounty는 바로 활성화하지 않고 MatchState 체크함)
				}
			}
			return true;
		}
		return false;
	};

	// 3v3 설정
	int32 Team0BotsToSpawn = 2; // 플레이어 1명 + 봇 2명
	int32 Team1BotsToSpawn = 3; // 적 팀 3명

	// 스폰 포인트를 셔플하여 랜덤하게 배정
	for (int32 i = 0; i < FoundSpawnPoints.Num(); i++)
	{
		int32 RandIndex = FMath::RandRange(i, FoundSpawnPoints.Num() - 1);
		FoundSpawnPoints.Swap(i, RandIndex);
	}

	for (AActor* Actor : FoundSpawnPoints)
	{
		ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor);
		if (!SP || SP->SpawnPointType != EBrawlSpawnPointType::Brawler) continue;

		if (SP->TeamID == 0 && Team0BotsToSpawn > 0)
		{
			if (SpawnBotForTeam(0, SP)) Team0BotsToSpawn--;
		}
		else if (SP->TeamID == 1 && Team1BotsToSpawn > 0)
		{
			if (SpawnBotForTeam(1, SP)) Team1BotsToSpawn--;
		}
	}
}