// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BrawlGameMode_Bounty.h"
#include "BrawlGameState_Bounty.h"
#include "BrawlPlayerState.h"
#include "BrawlCharacter.h"
#include "BrawlStarsTPSPlayerController.h"
#include "AI/BrawlAIController.h"
#include "GameMode/BrawlSpawnPoint.h"
#include "Environment/BrawlTieBreaker.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ABrawlGameMode_Bounty::ABrawlGameMode_Bounty()
{
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
		GS->SetMatchState(EBrawlMatchState::Playing);
		
		// 매치 타이머 시작
		GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ABrawlGameMode_Bounty::UpdateMatchTimer, 1.0f, true);
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
	Super::PostLogin(NewPlayer);

	if (ABrawlPlayerState* PS = NewPlayer->GetPlayerState<ABrawlPlayerState>())
	{
		if (PS->GetTeamID() == 255) PS->SetTeamID(0);
		AssignedTeams.Add(NewPlayer, 0); // 플레이어 팀 등록
		
		// 바운티 모드 초기 점수 설정
		PS->SetBounty(2);
		
		UE_LOG(LogTemp, Log, TEXT("BountyGameMode: PostLogin - Player [%s] TeamID: %d"), *NewPlayer->GetName(), PS->GetTeamID());
	}
}

AActor* ABrawlGameMode_Bounty::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!Player) return nullptr;

	int32 TeamID = -1;

	// 1. GameMode에서 직접 관리하는 팀 맵 확인 (가장 확실함)
	if (AssignedTeams.Contains(Player))
	{
		TeamID = AssignedTeams[Player];
	}
	// 2. PlayerState에서 확인
	else if (ABrawlPlayerState* PS = Player->GetPlayerState<ABrawlPlayerState>())
	{
		TeamID = PS->GetTeamID();
	}

	// 3. 플레이어 컨트롤러 기본값
	if ((TeamID == -1 || TeamID == 255) && Player->IsPlayerController())
	{
		TeamID = 0;
	}

	UE_LOG(LogTemp, Log, TEXT("BountyGameMode: ChoosePlayerStart for [%s], Final TeamID: %d"), *Player->GetName(), TeamID);

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);

	TArray<ABrawlSpawnPoint*> TeamSpawnPoints;
	TArray<ABrawlSpawnPoint*> ValidSpawnPoints;

	for (AActor* Actor : FoundSpawnPoints)
	{
		ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor);
		if (SP && SP->SpawnPointType == EBrawlSpawnPointType::Brawler && SP->TeamID == TeamID)
		{
			TeamSpawnPoints.Add(SP);

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
		UE_LOG(LogTemp, Log, TEXT("BountyGameMode: Found empty spot [%s] for Team %d"), *SelectedSpot->GetName(), TeamID);
	}
	else if (TeamSpawnPoints.Num() > 0)
	{
		SelectedSpot = TeamSpawnPoints[FMath::RandRange(0, TeamSpawnPoints.Num() - 1)];
		UE_LOG(LogTemp, Log, TEXT("BountyGameMode: All spots occupied. Forced [%s] for Team %d"), *SelectedSpot->GetName(), TeamID);
	}

	if (SelectedSpot) return SelectedSpot;

	UE_LOG(LogTemp, Error, TEXT("BountyGameMode: FAILED to find any SpawnPoint for Team %d! Falling back to engine default."), TeamID);
	return Super::ChoosePlayerStart_Implementation(Player);
}

UClass* ABrawlGameMode_Bounty::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (!InController) return nullptr;

	// 1. AI인 경우 할당된 클래스 반환
	if (AssignedAIClasses.Contains(InController))
	{
		UClass* FoundClass = AssignedAIClasses[InController];
		UE_LOG(LogTemp, Log, TEXT("BountyGameMode: GetDefaultPawnClass for AI Controller [%s] -> [%s]"), 
			*InController->GetName(), FoundClass ? *FoundClass->GetName() : TEXT("NULL"));
		return FoundClass;
	}

	// 2. 플레이어인 경우 부모 클래스(BrawlStarsTPSGameMode)의 로직 사용 (데이터 테이블 기반)
	UClass* DefaultClass = Super::GetDefaultPawnClassForController_Implementation(InController);
	UE_LOG(LogTemp, Log, TEXT("BountyGameMode: GetDefaultPawnClass for Player Controller [%s] -> [%s]"), 
		*InController->GetName(), DefaultClass ? *DefaultClass->GetName() : TEXT("NULL"));
	return DefaultClass;
}

void ABrawlGameMode_Bounty::SetupTeams()
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
					
					if (APawn* P = PC->GetPawn())
					{
						// (0,0,0)에 있거나 너무 낮게 스폰된 경우 재배치
						if (P->GetActorLocation().Z < -100.0f || P->GetActorLocation().IsNearlyZero(1.0f))
						{
							UE_LOG(LogTemp, Warning, TEXT("Brawler [%s] is at invalid location %s. Restarting..."), 
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
						// 폰이 아예 없다면 생성 요청
						RestartPlayer(PC);
					}
				}
			}
		}
	});
}

void ABrawlGameMode_Bounty::SpawnBots()
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

		// 1. AI 컨트롤러를 먼저 직접 스폰 (폰에 의해 자동 생성되지 않게 함)
		FActorSpawnParameters AICSpawnParams;
		AICSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// AIController 클래스가 지정되어 있지 않다면 ABrawlAIController를 기본으로 사용
		// 보통 Brawler 클래스에 설정된 AIControllerClass를 사용하는 것이 좋음
		UClass* AICClass = ABrawlAIController::StaticClass();
		if (BotClass.GetDefaultObject()->AIControllerClass)
		{
			AICClass = BotClass.GetDefaultObject()->AIControllerClass;
		}

		ABrawlAIController* AIC = GetWorld()->SpawnActor<ABrawlAIController>(AICClass, SP->GetActorLocation(), SP->GetActorRotation(), AICSpawnParams);
		
		if (AIC)
		{
			// AI 컨트롤러의 PlayerState를 즉시 생성 및 초기화
			AIC->InitPlayerState();

			// 리스폰 및 팀 판별을 위해 매핑 저장
			AssignedAIClasses.Add(AIC, BotClass);
			AssignedTeams.Add(AIC, TeamID);
			
			UE_LOG(LogTemp, Log, TEXT("BountyGameMode: Spawned AI Controller [%s] for Team %d. Assigned Class: %s"), 
				*AIC->GetName(), TeamID, *BotClass->GetName());

			// PlayerState에 팀 설정
			if (ABrawlPlayerState* PS = AIC->GetPlayerState<ABrawlPlayerState>())
			{
				PS->SetTeamID(TeamID);
				PS->SetBounty(2); // 바운티 모드 기본값 설정
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BountyGameMode: FAILED to initialize PlayerState for AI [%s]"), *AIC->GetName());
			}

			// 2. RestartPlayer를 통해 폰 스폰 (ChoosePlayerStart 로직을 타게 됨)
			// 현재 SP 위치를 직접 사용하기 위해 SP를 인자로 넘기는 RestartPlayerAtPlayerStart 사용 가능
			RestartPlayerAtPlayerStart(AIC, SP);

			// 스폰된 폰에 팀 ID 설정
			if (ABrawlCharacter* NewBot = Cast<ABrawlCharacter>(AIC->GetPawn()))
			{
				NewBot->SetGenericTeamId(FGenericTeamId(TeamID));
				
				if (GameModeAITree)
				{
					AIC->InjectGameModeSubtree(GameModeAITree);
				}

				// 행동 트리 시작
				AIC->SetAIActive(true);
			}
			return true;
		}
		return false;
	};

	int32 Team0BotsToSpawn = 2;
	int32 Team1BotsToSpawn = 3;

	// 스폰 포인트를 순회하며 가능한 곳에 스폰
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

void ABrawlGameMode_Bounty::NotifyKill(AActor* Killer, AActor* Victim)
{
	Super::NotifyKill(Killer, Victim);

	ABrawlCharacter* VictimChar = Cast<ABrawlCharacter>(Victim);
	if (!VictimChar) return;

	// Victim의 컨트롤러 미리 확보
	AController* VictimController = VictimChar->GetController();
	UE_LOG(LogTemp, Log, TEXT("BountyGameMode: NotifyKill - Victim: [%s], Controller: [%s]"), 
		*VictimChar->GetName(), VictimController ? *VictimController->GetName() : TEXT("NULL"));

	// Killer의 컨트롤러 확보
	AActor* KillerPawn = Killer;
	if (AController* KillerController = Cast<AController>(Killer))
	{
		KillerPawn = KillerController->GetPawn();
	}
	ABrawlCharacter* KillerChar = Cast<ABrawlCharacter>(KillerPawn);
	AController* KillerController = KillerChar ? KillerChar->GetController() : Cast<AController>(Killer);

	// PlayerState 찾기 (캐릭터 또는 컨트롤러 양쪽에서 시도)
	auto GetPS = [](AActor* Actor, AController* Controller) -> ABrawlPlayerState*
	{
		if (ABrawlCharacter* C = Cast<ABrawlCharacter>(Actor))
		{
			if (ABrawlPlayerState* PS = C->GetPlayerState<ABrawlPlayerState>()) return PS;
		}
		if (Controller)
		{
			return Controller->GetPlayerState<ABrawlPlayerState>();
		}
		return nullptr;
	};

	ABrawlPlayerState* KillerPS = GetPS(KillerChar, KillerController);
	ABrawlPlayerState* VictimPS = GetPS(VictimChar, VictimController);

	if (KillerPS && VictimPS)
	{
		int32 VictimBounty = VictimPS->GetBounty();
		int32 KillerTeam = KillerPS->GetTeamID();

		UE_LOG(LogTemp, Log, TEXT("BountyGameMode: Killer %s (Team %d) earned %d points from %s"), 
			*KillerPS->GetPlayerName(), KillerTeam, VictimBounty, *VictimPS->GetPlayerName());

		// 1. 킬러의 팀에 피해자의 현상금만큼 점수 추가
		if (ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>())
		{
			GS->AddTeamScore(KillerTeam, VictimBounty);
		}

		// 2. 킬러의 개인 현상금 증가 (+1, 최대 7)
		KillerPS->AddBounty(1);
		KillerPS->AddScoreContribution(VictimBounty);

		// 3. 피해자의 현상금 초기화
		VictimPS->ResetBounty();

		// 4. 타이 브레이커 강탈 로직
		if (VictimPS->HasTieBreaker())
		{
			// 피해자 해제
			VictimPS->SetHasTieBreaker(false);
			
			// 킬러에게 부여
			KillerPS->SetHasTieBreaker(true);
			TieBreakerOwnerState = KillerPS;

			// GameState 동기화
			if (ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>())
			{
				GS->SetTieBreakerTeam(KillerPS->GetTeamID());
			}

			UE_LOG(LogTemp, Log, TEXT("Tie Breaker STOLEN by %s (Team %d)"), *KillerPS->GetPlayerName(), KillerPS->GetTeamID());
		}

		// 5. 승리 조건 체크
		CheckWinCondition();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BountyGameMode: Failed to find PS. KillerPS: %s, VictimPS: %s"), 
			KillerPS ? TEXT("Valid") : TEXT("NULL"), 
			VictimPS ? TEXT("Valid") : TEXT("NULL"));
	}

	if (VictimController)
	{
		RequestRespawn(VictimController);
	}
}

void ABrawlGameMode_Bounty::RequestRespawn(AController* Controller)
{
	if (!Controller) return;

	// 리스폰에 필요한 정보를 미리 캡처
	TWeakObjectPtr<AController> WeakController(Controller);
	int32 TeamID = 0;
	if (ABrawlPlayerState* PS = Controller->GetPlayerState<ABrawlPlayerState>())
	{
		TeamID = PS->GetTeamID();
	}
	
	TSubclassOf<ABrawlCharacter> BrawlerClass = nullptr;
	if (AssignedAIClasses.Contains(Controller))
	{
		BrawlerClass = AssignedAIClasses[Controller];
	}

	UE_LOG(LogTemp, Log, TEXT("BountyGameMode: RequestRespawn for [%s] (Team %d) in %.1f seconds"), 
		*Controller->GetName(), TeamID, RespawnDelay);

	FTimerHandle RespawnTimerHandle;
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, [this, WeakController, TeamID, BrawlerClass]()
	{
		if (WeakController.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("BountyGameMode: Respawn Timer Fired for [%s]. Calling RespawnBrawler."), *WeakController->GetName());
			RespawnBrawler(WeakController.Get());
		}
		else if (BrawlerClass != nullptr)
		{
			// 컨트롤러가 소실된 경우 (AI 전용 복구 로직)
			UE_LOG(LogTemp, Warning, TEXT("BountyGameMode: Controller lost. Re-spawning AI for Team %d"), TeamID);
			
			// 1. 직접 스폰 지점 찾기 (RestartPlayer에게 맡기지 않고 명시적으로 찾음)
			AActor* FoundSP = nullptr;
			TArray<AActor*> FoundSpawnPoints;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);
			for (AActor* Actor : FoundSpawnPoints)
			{
				ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor);
				if (SP && SP->SpawnPointType == EBrawlSpawnPointType::Brawler && SP->TeamID == TeamID)
				{
					FoundSP = SP;
					break; 
				}
			}

			if (!FoundSP)
			{
				UE_LOG(LogTemp, Error, TEXT("BountyGameMode: Backup Respawn failed - No SpawnPoint found for Team %d"), TeamID);
				return;
			}

			FActorSpawnParameters AICSpawnParams;
			AICSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			UClass* AICClass = ABrawlAIController::StaticClass();
			if (BrawlerClass.GetDefaultObject()->AIControllerClass)
			{
				AICClass = BrawlerClass.GetDefaultObject()->AIControllerClass;
			}

			if (ABrawlAIController* NewAIC = GetWorld()->SpawnActor<ABrawlAIController>(AICClass, FoundSP->GetActorLocation(), FoundSP->GetActorRotation(), AICSpawnParams))
			{
				AssignedAIClasses.Add(NewAIC, BrawlerClass);
				AssignedTeams.Add(NewAIC, TeamID);
				
				if (ABrawlPlayerState* PS = NewAIC->GetPlayerState<ABrawlPlayerState>())
				{
					PS->SetTeamID(TeamID);
				}

				RestartPlayerAtPlayerStart(NewAIC, FoundSP);

				if (APawn* NewPawn = NewAIC->GetPawn())
				{
					FVector SpawnLocation = FoundSP->GetActorLocation() + FVector(0, 0, 95.0f);
					NewPawn->SetActorLocationAndRotation(SpawnLocation, FoundSP->GetActorRotation());

					if (ABrawlCharacter* NewBot = Cast<ABrawlCharacter>(NewPawn))
					{
						NewBot->SetGenericTeamId(FGenericTeamId(TeamID));

						if (GameModeAITree) NewAIC->InjectGameModeSubtree(GameModeAITree);

						// 매치 가 진행 중일 때만 행동 트리 시작
						if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
						{
							if (GS->IsMatchInProgress())
							{
								NewAIC->SetAIActive(true);
							}
						}
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("BountyGameMode: Respawn failed. Controller and Class info both missing."));
		}
	}, RespawnDelay, false);
}

void ABrawlGameMode_Bounty::RespawnBrawler(AController* Controller)
{
	if (!Controller) return;

	UE_LOG(LogTemp, Log, TEXT("BountyGameMode: RespawnBrawler starting for [%s]"), *Controller->GetName());

	ABrawlCharacter* Character = Cast<ABrawlCharacter>(Controller->GetPawn());
	
	// 1. 만약 폰이 소실되었다면 RestartPlayer로 새로 생성
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("BountyGameMode: Pawn lost for [%s]. Restarting player."), *Controller->GetName());
		RestartPlayer(Controller);
		Character = Cast<ABrawlCharacter>(Controller->GetPawn());
	}

	// 2. 캐릭터 리스폰 처리 (위치 이동 및 활성화)
	if (Character)
	{
		AActor* SpawnSpot = FindPlayerStart(Controller);
		FVector SpawnLoc = SpawnSpot ? SpawnSpot->GetActorLocation() : FVector::ZeroVector;
		FRotator SpawnRot = SpawnSpot ? SpawnSpot->GetActorRotation() : FRotator::ZeroRotator;

		Character->RespawnAt(SpawnLoc + FVector(0, 0, 95.0f), SpawnRot);

		// AI 행동 트리 재시작 (매치 진행 중일 때만)
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

	ABrawlPlayerState* PS = Picker->GetPlayerState<ABrawlPlayerState>();
	if (PS)
	{
		// 1. 기존 소유자 해제
		if (TieBreakerOwnerState)
		{
			TieBreakerOwnerState->SetHasTieBreaker(false);
		}

		// 2. 새 소유자 설정
		PS->SetHasTieBreaker(true);
		TieBreakerOwnerState = PS;

		// 3. 팀 점수 1점 추가 및 GameState 동기화
		if (ABrawlGameState_Bounty* GS = GetGameState<ABrawlGameState_Bounty>())
		{
			GS->AddTeamScore(PS->GetTeamID(), 1);
			GS->SetTieBreakerTeam(PS->GetTeamID());
		}

		UE_LOG(LogTemp, Log, TEXT("Tie Breaker picked up by Team %d (+1 Point)"), PS->GetTeamID());
	}
}

void ABrawlGameMode_Bounty::SpawnTieBreaker()
{
	if (!TieBreakerClass) return;

	// 맵 중앙에 스폰 (0,0,0 가정 혹은 특정 스폰 포인트)
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

	// 플레이어 팀 ID 확인 (보통 0)
	int32 PlayerTeam = 0;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ABrawlPlayerState* PS = PC->GetPlayerState<ABrawlPlayerState>())
		{
			PlayerTeam = PS->GetTeamID();
		}
	}

	// 1. 점수 선점 체크
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

	// 2. 시간 종료 체크
	if (GS->GetRemainingTime() <= 0)
	{
		int32 WinningTeam = 0;
		if (Score0 > Score1) WinningTeam = 0;
		else if (Score1 > Score0) WinningTeam = 1;
		else
		{
			// 동점 시 타이 브레이커 체크
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

	UE_LOG(LogTemp, Log, TEXT("Bounty Match Over! Winning Team: %d"), WinningTeam);
}
