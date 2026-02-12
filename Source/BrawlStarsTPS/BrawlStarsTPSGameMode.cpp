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

void ABrawlStarsTPSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

UClass* ABrawlStarsTPSGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// 플레이어 컨트롤러인 경우 GameInstance에서 선택한 브롤러 확인
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
	int32 TargetTeamID = -1;
	if (ABrawlPlayerState* PS = Player->GetPlayerState<ABrawlPlayerState>())
	{
		TargetTeamID = PS->GetTeamID();
	}

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);
	
	TArray<ABrawlSpawnPoint*> ValidSpawnPoints;
	for (AActor* Actor : FoundSpawnPoints)
	{
		if (ABrawlSpawnPoint* SpawnPoint = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SpawnPoint->SpawnPointType == EBrawlSpawnPointType::Brawler)
			{
				// 팀 ID가 지정된 경우 필터링
				if (TargetTeamID != -1 && SpawnPoint->TeamID != TargetTeamID && SpawnPoint->TeamID != 255)
				{
					continue;
				}

				// 신규 IsOccupied 함수 사용
				if (!SpawnPoint->IsOccupied(100.0f))
				{
					ValidSpawnPoints.Add(SpawnPoint);
				}
			}
		}
	}

	if (ValidSpawnPoints.Num() > 0)
	{
		int32 RandIndex = FMath::RandRange(0, ValidSpawnPoints.Num() - 1);
		return ValidSpawnPoints[RandIndex];
	}

	// 모든 지점이 점유되었다면 팀 포인트 중 하나 랜덤 선택 (백업)
	TArray<ABrawlSpawnPoint*> FallbackPoints;
	for (AActor* Actor : FoundSpawnPoints)
	{
		if (ABrawlSpawnPoint* SP = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SP->SpawnPointType == EBrawlSpawnPointType::Brawler && (TargetTeamID == -1 || SP->TeamID == TargetTeamID || SP->TeamID == 255))
			{
				FallbackPoints.Add(SP);
			}
		}
	}

	if (FallbackPoints.Num() > 0)
	{
		return FallbackPoints[FMath::RandRange(0, FallbackPoints.Num() - 1)];
	}

	return Super::ChoosePlayerStart_Implementation(Player);
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

void ABrawlStarsTPSGameMode::SpawnBots()
{
	if (AICharacterClasses.Num() == 0 || MaxBots <= 0) return;

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);
	
	// Shuffle spawn points
	for (int32 i = 0; i < FoundSpawnPoints.Num(); i++)
	{
		int32 RandIndex = FMath::RandRange(i, FoundSpawnPoints.Num() - 1);
		FoundSpawnPoints.Swap(i, RandIndex);
	}

	int32 SpawnedCount = 0;
	for (AActor* Actor : FoundSpawnPoints)
	{
		if (SpawnedCount >= MaxBots) break;

		if (ABrawlSpawnPoint* SpawnPoint = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SpawnPoint->SpawnPointType == EBrawlSpawnPointType::Brawler)
			{
				// 신규 IsOccupied 함수 사용
				if (SpawnPoint->IsOccupied(100.0f)) continue;

				TSubclassOf<ABrawlCharacter> BotClass = AICharacterClasses[FMath::RandRange(0, AICharacterClasses.Num() - 1)];
				if (BotClass)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
					
					ABrawlCharacter* NewBot = GetWorld()->SpawnActor<ABrawlCharacter>(BotClass, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation(), SpawnParams);
					if (NewBot)
					{
						NewBot->SpawnDefaultController();
						ConfigureAI(NewBot->GetController(), SpawnPoint->TeamID);
						SpawnedCount++;
					}
				}
			}
		}
	}
}

void ABrawlStarsTPSGameMode::ConfigureAI(AController* AIController, int32 TeamID)
{
	if (AIController == nullptr) return;

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
