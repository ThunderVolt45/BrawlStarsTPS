#include "BrawlStarsTPSGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "BrawlGameState.h"
#include "BrawlGameInstance.h"
#include "BrawlCharacter.h"
#include "Data/BrawlerClassData.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/BrawlSpawnPoint.h"
#include "BrawlPlayerState.h"
#include "AI/BrawlAIController.h"
#include "Environment/BrawlPoisonZone.h"
#include "AbilitySystemComponent.h"
#include "BrawlStarsTPSPlayerController.h"

ABrawlStarsTPSGameMode::ABrawlStarsTPSGameMode()
{
	// GameState 및 PlayerState 클래스 설정
	GameStateClass = ABrawlGameState::StaticClass();
	PlayerStateClass = ABrawlPlayerState::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PrimaryActorTick.bCanEverTick = true;
}

void ABrawlStarsTPSGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		// 스트링 테이블에서 게임 모드 정보 로드
		if (!GameModeStringTable.IsNull())
		{
			// 에셋의 경로 자체가 스트링 테이블의 고유 ID가 됩니다.
			FName TableID = FName(*GameModeStringTable.ToSoftObjectPath().GetAssetPathString());
			FString BaseKey = GameModeID.ToString();

			FText ModeName = FText::FromStringTable(TableID, BaseKey + TEXT("_Name"));
			FText ModeDesc = FText::FromStringTable(TableID, BaseKey + TEXT("_Desc"));

			GS->SetModeInfo(ModeName, ModeDesc);
			
			UE_LOG(LogTemp, Log, TEXT("GameMode: Loaded strings from Table [%s] using ID [%s]"), 
				*GameModeStringTable.ToString(), *TableID.ToString());
		}

		GS->SetMatchState(EBrawlMatchState::Intro);

		FTimerHandle StartTimerHandle;
		GetWorldTimerManager().SetTimer(StartTimerHandle, this, &ABrawlStarsTPSGameMode::StartMatch, StartDelay, false);
	}
}

void ABrawlStarsTPSGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

int32 ABrawlStarsTPSGameMode::GetControllerTeamID(AController* InController) const
{
	if (!InController) return 255;

	// 1. AI 컨트롤러 또는 이미 명시적으로 할당된 팀 맵에서 확인
	if (AssignedTeams.Contains(InController))
	{
		return AssignedTeams[InController];
	}

	// 2. 플레이어 컨트롤러인데 맵에 없다면, 게임 모드별 규칙 적용
	if (InController->IsPlayerController())
	{
		if (GameModeID == FName("Bounty") || GameModeID == FName("Knockout"))
		{
			return 0; // Bounty/Knockout 플레이어는 무조건 팀 0
		}
	}

	// 3. PlayerState가 생성되어 있다면 거기서 확인
	if (ABrawlPlayerState* PS = InController->GetPlayerState<ABrawlPlayerState>())
	{
		return PS->GetTeamID();
	}

	return 255;
}

void ABrawlStarsTPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	// 1. 하위 클래스에서 이미 AssignedTeams에 등록했을 수도 있음
	int32 InitialTeam = 255;
	if (AssignedTeams.Contains(NewPlayer))
	{
		InitialTeam = AssignedTeams[NewPlayer];
	}

	Super::PostLogin(NewPlayer);

	// 2. Super::PostLogin 이후에는 PlayerState가 반드시 존재해야 함
	if (ABrawlPlayerState* PS = NewPlayer->GetPlayerState<ABrawlPlayerState>())
	{
		// 이미 설정된 팀이 있다면 적용, 없다면 기본값 255
		if (InitialTeam != 255)
		{
			PS->SetTeamID(InitialTeam);
			UE_LOG(LogTemp, Log, TEXT("GameMode: PostLogin - Player [%s] assigned with Pre-determined TeamID: %d"), *NewPlayer->GetName(), InitialTeam);
		}
		else if (PS->GetTeamID() == 255)
		{
			// 게임 모드별 기본 플레이어 팀 할당
			int32 DefaultTeam = 255;
			if (GameModeID == FName("Bounty") || GameModeID == FName("Knockout"))
			{
				DefaultTeam = 0;
			}
			
			PS->SetTeamID(DefaultTeam);
			AssignedTeams.Add(NewPlayer, DefaultTeam);
			UE_LOG(LogTemp, Log, TEXT("GameMode: PostLogin - Player [%s] initialized with default TeamID: %d"), *NewPlayer->GetName(), DefaultTeam);
		}
		else
		{
			// PlayerState에 이미 무언가 설정되어 있다면 맵에 동기화
			AssignedTeams.Add(NewPlayer, PS->GetTeamID());
			UE_LOG(LogTemp, Log, TEXT("GameMode: PostLogin - Player [%s] already has TeamID: %d"), *NewPlayer->GetName(), PS->GetTeamID());
		}
	}
}

UClass* ABrawlStarsTPSGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (!InController) return nullptr;

	// 1. 이미 할당된 클래스가 있다면 (AI 또는 특별 관리되는 컨트롤러) 반환
	if (AssignedAIClasses.Contains(InController))
	{
		return AssignedAIClasses[InController];
	}

	// 2. AI 컨트롤러인데 할당된 클래스가 없는 경우 랜덤 선택 (안전장치)
	if (InController->IsA<AAIController>() && AICharacterClasses.Num() > 0)
	{
		int32 CharIndex = FMath::RandRange(0, AICharacterClasses.Num() - 1);
		TSubclassOf<ABrawlCharacter> RandomClass = AICharacterClasses[CharIndex];
		AssignedAIClasses.Add(InController, RandomClass);
		return RandomClass;
	}

	// 3. 플레이어 컨트롤러인 경우 GameInstance에서 선택한 브롤러 확인
	if (APlayerController* PC = Cast<APlayerController>(InController))
	{
		if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
		{
			if (BrawlerClassDataTable)
			{
				FName RowName = GI->SelectedBrawlerRowName;
				static const FString ContextString(TEXT("GetDefaultPawnClass"));
				FBrawlerClassData* Row = BrawlerClassDataTable->FindRow<FBrawlerClassData>(RowName, ContextString);

				if (Row && Row->BrawlerClass)
				{
					return Row->BrawlerClass;
				}
			}
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* ABrawlStarsTPSGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!Player) return nullptr;

	// GetControllerTeamID를 사용하여 스폰 시점에 맵 등록 여부와 상관없이 올바른 팀 판별
	int32 TeamID = GetControllerTeamID(Player);

	UE_LOG(LogTemp, Log, TEXT("ChoosePlayerStart: Player [%s], Determined TeamID: %d"), *Player->GetName(), TeamID);

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);
	
	TArray<ABrawlSpawnPoint*> TeamSpawnPoints;
	TArray<ABrawlSpawnPoint*> ValidSpawnPoints;

	for (AActor* Actor : FoundSpawnPoints)
	{
		if (ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SP->SpawnPointType == EBrawlSpawnPointType::Brawler)
			{
				// 팀 ID 필터링 (유효한 팀 ID가 있는 경우 해당 팀의 포인트 또는 공용 포인트만 허용)
				if (TeamID != 255)
				{
					if (SP->TeamID != TeamID && SP->TeamID != 255) continue;
				}

				TeamSpawnPoints.Add(SP);

				// 점유 상태 확인
				if (!SP->IsOccupied(100.0f))
				{
					ValidSpawnPoints.Add(SP);
				}
			}
		}
	}

	if (ValidSpawnPoints.Num() > 0)
	{
		AActor* Selected = ValidSpawnPoints[FMath::RandRange(0, ValidSpawnPoints.Num() - 1)];
		UE_LOG(LogTemp, Log, TEXT("ChoosePlayerStart: Selected Valid SP [%s] for Team %d"), *Selected->GetName(), TeamID);
		return Selected;
	}
	
	if (TeamSpawnPoints.Num() > 0)
	{
		AActor* Selected = TeamSpawnPoints[FMath::RandRange(0, TeamSpawnPoints.Num() - 1)];
		UE_LOG(LogTemp, Log, TEXT("ChoosePlayerStart: Selected Team SP [%s] (Occupied) for Team %d"), *Selected->GetName(), TeamID);
		return Selected;
	}

	UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart: No suitable BrawlSpawnPoint found for Team %d, falling back to Super"), TeamID);
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ABrawlStarsTPSGameMode::SetupTeams()
{
}

void ABrawlStarsTPSGameMode::SpawnBots()
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
				if (SpawnBotAt(SP->TeamID, SP))
				{
					SpawnedCount++;
				}
			}
		}
	}
}

bool ABrawlStarsTPSGameMode::SpawnBotAt(int32 TeamID, ABrawlSpawnPoint* SP)
{
	if (!SP || SP->IsOccupied(50.0f)) return false;

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

		if (ABrawlCharacter* NewBot = Cast<ABrawlCharacter>(AIC->GetPawn()))
		{
			ConfigureAI(AIC, TeamID);
		}
		return true;
	}
	return false;
}

void ABrawlStarsTPSGameMode::ConfigureAI(AController* AIController, int32 TeamID)
{
	if (!AIController) return;

	if (ABrawlCharacter* BrawlChar = Cast<ABrawlCharacter>(AIController->GetPawn()))
	{
		BrawlChar->SetGenericTeamId(FGenericTeamId(TeamID));
	}

	if (ABrawlAIController* BrawlAI = Cast<ABrawlAIController>(AIController))
	{
		if (GameModeAITree)
		{
			BrawlAI->InjectGameModeSubtree(GameModeAITree);
		}
	}
}

void ABrawlStarsTPSGameMode::StartMatch()
{
	if (bHasMatchStarted) return;
	bHasMatchStarted = true;

	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		// 1. 먼저 MatchStart 상태로 전이 (START 연출)
		GS->SetMatchState(EBrawlMatchState::MatchStart);

		// 2. 연출 시간(1.5초) 뒤에 실제 Playing 상태로 전이
		FTimerHandle PlayingStateTimerHandle;
		GetWorldTimerManager().SetTimer(PlayingStateTimerHandle, [this, GS]()
		{
			if (GS)
			{
				GS->SetMatchState(EBrawlMatchState::Playing);
			}
		}, 1.5f, false);
	}
}

void ABrawlStarsTPSGameMode::EndGame(bool bIsPlayerWinner, int32 RankOrTeam)
{
	GetWorld()->GetTimerManager().ClearTimer(PoisonUpdateTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(PoisonDamageTimerHandle);

	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		GS->SetMatchState(EBrawlMatchState::GameOver);
	}

	if (ABrawlStarsTPSPlayerController* PC = Cast<ABrawlStarsTPSPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->ShowMatchResultUI(bIsPlayerWinner, RankOrTeam);
	}
}

void ABrawlStarsTPSGameMode::NotifyKill(AActor* Killer, AActor* Victim)
{
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		GS->NotifyBrawlerKilled(Killer, Victim);
	}
	
	UE_LOG(LogGameMode, Log, TEXT("Kill Confirmed: [%s] killed [%s]"), 
		Killer ? *Killer->GetName() : TEXT("Environment"), 
		Victim ? *Victim->GetName() : TEXT("Unknown"));
}

void ABrawlStarsTPSGameMode::StartPoisonLogic()
{
	CurrentSafeZoneRadius = InitialSafeZoneRadius;

	if (PoisonZoneClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PoisonZoneInstance = GetWorld()->SpawnActor<ABrawlPoisonZone>(PoisonZoneClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (PoisonZoneInstance)
		{
			PoisonZoneInstance->SetZoneRadius(CurrentSafeZoneRadius);
		}
	}

	if (PoisonStartDelay > 0.0f)
	{
		FTimerHandle StartDelayHandle;
		GetWorldTimerManager().SetTimer(StartDelayHandle, [this]()
		{
			GetWorldTimerManager().SetTimer(PoisonUpdateTimerHandle, this, &ABrawlStarsTPSGameMode::UpdatePoisonZone, 0.1f, true);
			GetWorldTimerManager().SetTimer(PoisonDamageTimerHandle, this, &ABrawlStarsTPSGameMode::CheckPoisonDamage, PoisonDamageInterval, true);
		}, PoisonStartDelay, false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(PoisonUpdateTimerHandle, this, &ABrawlStarsTPSGameMode::UpdatePoisonZone, 0.1f, true);
		GetWorldTimerManager().SetTimer(PoisonDamageTimerHandle, this, &ABrawlStarsTPSGameMode::CheckPoisonDamage, PoisonDamageInterval, true);
	}
}

void ABrawlStarsTPSGameMode::UpdatePoisonZone()
{
	CurrentSafeZoneRadius -= PoisonShrinkSpeed * 0.1f;
	if (CurrentSafeZoneRadius < MinSafeZoneRadius)
	{
		CurrentSafeZoneRadius = MinSafeZoneRadius;
	}

	if (PoisonZoneInstance)
	{
		PoisonZoneInstance->SetZoneRadius(CurrentSafeZoneRadius);
	}
}

void ABrawlStarsTPSGameMode::CheckPoisonDamage()
{
	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);

	for (AActor* Actor : FoundBrawlers)
	{
		if (ABrawlCharacter* Brawler = Cast<ABrawlCharacter>(Actor))
		{
			if (Brawler->IsDead()) continue;

			FVector Loc = Brawler->GetActorLocation();
			float DistX = FMath::Abs(Loc.X);
			float DistY = FMath::Abs(Loc.Y);
			
			if (DistX > CurrentSafeZoneRadius || DistY > CurrentSafeZoneRadius)
			{
				if (UAbilitySystemComponent* ASC = Brawler->GetAbilitySystemComponent())
				{
					FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
					ContextHandle.AddInstigator(PoisonZoneInstance, PoisonZoneInstance);

					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PoisonDamageEffectClass, 1.0f, ContextHandle);
					if (SpecHandle.IsValid())
					{
						SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), PoisonDamage);
						ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}
		}
	}
}
