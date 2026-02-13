// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BrawlGameMode_Bounty.h"
#include "BrawlGameState_Bounty.h"
#include "BrawlPlayerState.h"
#include "BrawlCharacter.h"
#include "AI/BrawlAIController.h"
#include "GameMode/BrawlSpawnPoint.h"
#include "Kismet/GameplayStatics.h"

ABrawlGameMode_Bounty::ABrawlGameMode_Bounty()
{
	GameModeType = EBrawlGameModeType::Bounty;
	GameStateClass = ABrawlGameState_Bounty::StaticClass();
	PlayerStateClass = ABrawlPlayerState::StaticClass();
	PrimaryActorTick.bCanEverTick = true;

	MatchDuration = 120;
	TargetScore = 20;
	RespawnDelay = 3.0f;
}

void ABrawlGameMode_Bounty::BeginPlay()
{
	Super::BeginPlay();

	SetupTeams();
	SpawnBots();
	SpawnTieBreaker();

	if (ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>())
	{
		GS->SetRemainingTime(MatchDuration);
		GS->SetMatchState(EBrawlMatchState::Intro);

		FTimerHandle StartTimerHandle;
		GetWorldTimerManager().SetTimer(StartTimerHandle, this, &ABrawlGameMode_Bounty::StartMatch, 5.0f, false);
	}
}

void ABrawlGameMode_Bounty::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABrawlGameMode_Bounty::StartMatch()
{
	if (bHasMatchStarted) return;
	bHasMatchStarted = true;

	if (ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>())
	{
		// 1. MatchStart 상태로 전이
		GS->SetMatchState(EBrawlMatchState::MatchStart);

		// 2. 1.5초 뒤 Playing 상태로 전이 및 타이머 시작
		FTimerHandle PlayingStateTimerHandle;
		GetWorldTimerManager().SetTimer(PlayingStateTimerHandle, [this, GS]()
		{
			if (GS)
			{
				GS->SetMatchState(EBrawlMatchState::Playing);
				// 매치 타이머 시작
				GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ABrawlGameMode_Bounty::UpdateMatchTimer, 1.0f, true);
			}
		}, 1.5f, false);
	}
}

void ABrawlGameMode_Bounty::UpdateMatchTimer()
{
	ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>();
	if (!GS) return;

	int32 NewTime = GS->GetRemainingTime() - 1;
	GS->SetRemainingTime(NewTime);

	if (NewTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		CheckWinCondition(); // 시간 종료 시 승리 체크
	}
}

void ABrawlGameMode_Bounty::PostLogin(APlayerController* NewPlayer)
{
	// 1. Super::PostLogin 호출 전에 맵에 먼저 등록 (ChoosePlayerStart에서 참조할 수 있도록)
	if (NewPlayer)
	{
		AssignedTeams.Add(NewPlayer, 0);
	}

	// 2. 부모 클래스 호출 (여기서 InitPlayerState 및 RestartPlayer가 실행됨)
	Super::PostLogin(NewPlayer);

	// 3. 이제 생성되었을 PlayerState에 팀 및 바운티 설정
	if (ABrawlPlayerState* PS = NewPlayer->GetPlayerState<ABrawlPlayerState>())
	{
		PS->SetTeamID(0);
		PS->SetBounty(2);
	}
}

AActor* ABrawlGameMode_Bounty::ChoosePlayerStart_Implementation(AController* Player)
{
	// 부모 클래스의 팀 기반 선택 로직 사용
	return Super::ChoosePlayerStart_Implementation(Player);
}

UClass* ABrawlGameMode_Bounty::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// 부모 클래스의 데이터 테이블/할당 기반 로직 사용
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ABrawlGameMode_Bounty::SetupTeams()
{
	// Bounty 전용 팀 할당 (3v3) 수행
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (ABrawlPlayerState* PS = PC->GetPlayerState<ABrawlPlayerState>())
			{
				// 플레이어는 무조건 팀 0
				PS->SetTeamID(0);
				AssignedTeams.Add(PC, 0);
				PS->SetBounty(2);
				
				APawn* P = PC->GetPawn();
				bool bNeedsRestart = false;

				if (!P)
				{
					bNeedsRestart = true;
				}
				else
				{
					// 현재 위치가 비정상적이거나 (Z < -100) 너무 0에 가깝거나
					if (P->GetActorLocation().Z < -100.0f || P->GetActorLocation().IsNearlyZero(1.0f))
					{
						bNeedsRestart = true;
					}
					else
					{
						// 현재 위치 주변의 스폰 포인트를 찾아 팀 확인
						TArray<AActor*> NearbySpawnPoints;
						TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
						ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic)); // 스폰 포인트는 대개 Static 또는 Dynamic
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

void ABrawlGameMode_Bounty::SpawnBots()
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

	// 모든 스폰된 AI에게 바운티 기본값 2 부여
	for (auto& Pair : AssignedTeams)
	{
		if (Pair.Key->IsA<AAIController>())
		{
			if (ABrawlPlayerState* PS = Pair.Key->GetPlayerState<ABrawlPlayerState>())
			{
				PS->SetBounty(2);
			}
		}
	}
}

void ABrawlGameMode_Bounty::NotifyKill(AActor* Killer, AActor* Victim)
{
	Super::NotifyKill(Killer, Victim);

	ABrawlCharacter* VictimChar = Cast<ABrawlCharacter>(Victim);
	if (!VictimChar) return;

	AController* VictimController = VictimChar->GetController();
	
	// Killer 정보 확보
	AActor* KillerPawn = Killer;
	if (AController* KillerController = Cast<AController>(Killer))
	{
		KillerPawn = KillerController->GetPawn();
	}
	ABrawlCharacter* KillerChar = Cast<ABrawlCharacter>(KillerPawn);
	AController* KillerController = KillerChar ? KillerChar->GetController() : Cast<AController>(Killer);

	ABrawlPlayerState* KillerPS = KillerController ? KillerController->GetPlayerState<ABrawlPlayerState>() : nullptr;
	ABrawlPlayerState* VictimPS = VictimController ? VictimController->GetPlayerState<ABrawlPlayerState>() : nullptr;

	if (KillerPS && VictimPS)
	{
		// 점수 집계는 매치가 진행 중일 때만 수행
		if (ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>())
		{
			if (GS->GetMatchState() == EBrawlMatchState::Playing)
			{
				int32 VictimBounty = VictimPS->GetBounty();
				int32 KillerTeam = KillerPS->GetTeamID();

				GS->AddTeamScore(KillerTeam, VictimBounty);

				KillerPS->AddBounty(1);
				KillerPS->AddScoreContribution(VictimBounty);
				VictimPS->ResetBounty();

				if (VictimPS->HasTieBreaker())
				{
					VictimPS->SetHasTieBreaker(false);
					KillerPS->SetHasTieBreaker(true);
					TieBreakerOwnerState = KillerPS;

					GS->SetTieBreakerTeam(KillerPS->GetTeamID());
				}
			}
		}

		CheckWinCondition();
	}

	if (VictimController)
	{
		RequestRespawn(VictimController);
	}
}

void ABrawlGameMode_Bounty::RequestRespawn(AController* Controller)
{
	if (!Controller) return;

	TWeakObjectPtr<AController> WeakController(Controller);
	
	FTimerHandle RespawnTimerHandle;
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, [this, WeakController]()
	{
		if (WeakController.IsValid())
		{
			RespawnBrawler(WeakController.Get());
		}
	}, RespawnDelay, false);
}

void ABrawlGameMode_Bounty::RespawnBrawler(AController* Controller)
{
	if (!Controller) return;

	ABrawlCharacter* Character = Cast<ABrawlCharacter>(Controller->GetPawn());
	
	if (!Character)
	{
		RestartPlayer(Controller);
		Character = Cast<ABrawlCharacter>(Controller->GetPawn());
	}

	if (Character)
	{
		AActor* SpawnSpot = FindPlayerStart(Controller);
		FVector SpawnLoc = SpawnSpot ? SpawnSpot->GetActorLocation() : FVector::ZeroVector;
		FRotator SpawnRot = SpawnSpot ? SpawnSpot->GetActorRotation() : FRotator::ZeroRotator;

		Character->RespawnAt(SpawnLoc, SpawnRot);

		if (ABrawlAIController* AIC = Cast<ABrawlAIController>(Controller))
		{
			if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
			{
				if (GS->IsMatchInProgress())
				{
					AIC->SetAIActive(true);
				}
			}
		}
	}

	if (ABrawlPlayerState* PS = Controller->GetPlayerState<ABrawlPlayerState>())
	{
		PS->SetBounty(2);
	}
}

void ABrawlGameMode_Bounty::OnTieBreakerPickedUp(ABrawlCharacter* Picker)
{
	if (!Picker) return;

	// 매치 진행 중일 때만 처리
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		if (GS->GetMatchState() != EBrawlMatchState::Playing) return;
	}

	ABrawlPlayerState* PS = Picker->GetPlayerState<ABrawlPlayerState>();
	if (PS)
	{
		if (TieBreakerOwnerState)
		{
			TieBreakerOwnerState->SetHasTieBreaker(false);
		}

		PS->SetHasTieBreaker(true);
		TieBreakerOwnerState = PS;

		if (ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>())
		{
			GS->AddTeamScore(PS->GetTeamID(), 1);
			GS->SetTieBreakerTeam(PS->GetTeamID());
		}
	}
}

void ABrawlGameMode_Bounty::SpawnTieBreaker()
{
	if (!TieBreakerClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(TieBreakerClass, FVector(0, 0, 100.0f), FRotator::ZeroRotator, SpawnParams);
}

void ABrawlGameMode_Bounty::CheckWinCondition()
{
	ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>();
	if (!GS) return;

	int32 Score0 = GS->GetTeamScore(0);
	int32 Score1 = GS->GetTeamScore(1);

	int32 PlayerTeam = 0;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ABrawlPlayerState* PS = PC->GetPlayerState<ABrawlPlayerState>())
		{
			PlayerTeam = PS->GetTeamID();
		}
	}

	if (Score0 >= TargetScore)
	{
		EndGame(PlayerTeam == 0, 0);
		return;
	}
	if (Score1 >= TargetScore)
	{
		EndGame(PlayerTeam == 1, 1);
		return;
	}

	if (GS->GetRemainingTime() <= 0)
	{
		int32 WinningTeam = 0;
		if (Score0 > Score1) WinningTeam = 0;
		else if (Score1 > Score0) WinningTeam = 1;
		else
		{
			if (TieBreakerOwnerState)
			{
				WinningTeam = TieBreakerOwnerState->GetTeamID();
			}
		}
		EndGame(PlayerTeam == WinningTeam, WinningTeam);
	}
}

void ABrawlGameMode_Bounty::EndGame(bool bIsPlayerWinner, int32 WinningTeam)
{
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	Super::EndGame(bIsPlayerWinner, WinningTeam);
}
