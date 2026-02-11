// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BrawlGameMode_Showdown.h"
#include "Environment/BrawlPowerCubeBox.h"
#include "Environment/BrawlPowerCube.h" 
#include "Environment/BrawlPoisonZone.h" // 추가
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
	// 기본 설정
	GameStateClass = ABrawlGameState::StaticClass();
	PlayerStateClass = ABrawlPlayerState::StaticClass();
	
	MaxPowerCubeBoxes = 15;
	MaxBots = 5;
	PrimaryActorTick.bCanEverTick = true; // Tick 활성화
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

	// GameState 동기화 및 매치 상태 설정
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		GS->SetAliveBrawlerCount(AliveBrawlerCount);

		// 1. 인트로 단계 시작 (클라이언트로 전파되어 연출 트리거)
		GS->SetMatchState(EBrawlMatchState::Intro);

		// 2. 일정 시간 뒤 게임 시작 (연출 시간 확보)
		FTimerHandle StartTimerHandle;
		GetWorldTimerManager().SetTimer(StartTimerHandle, this, &ABrawlGameMode_Showdown::StartMatch, 5.0f, false);
	}
}

void ABrawlGameMode_Showdown::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABrawlGameMode_Showdown::StartMatch()
{
	if (bHasMatchStarted) return;

	bHasMatchStarted = true;

	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		// 상태를 Playing으로 변경 (AI 활성화 및 클라 연출 트리거)
		GS->SetMatchState(EBrawlMatchState::Playing);
	}

	// 독구름 로직 시작
	StartPoisonLogic();

	UE_LOG(LogTemp, Log, TEXT("Showdown Match Officially Started!"));
}

void ABrawlGameMode_Showdown::HandleMatchStartCountdown(float DeltaTime)
{
	// MatchFlowComponent와 GameState의 상태 변화로 대체됨
}

void ABrawlGameMode_Showdown::StartPoisonLogic()
{
	CurrentSafeZoneRadius = InitialSafeZoneRadius;

	// 독구름 액터 스폰 (맵 중앙 0,0,0 가정)
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

	// 축소 및 데미지 타이머 설정
	if (PoisonStartDelay > 0.0f)
	{
		FTimerHandle StartDelayHandle;
		GetWorld()->GetTimerManager().SetTimer(StartDelayHandle, [this]()
		{
			// 0.1초마다 구역 축소 업데이트
			GetWorld()->GetTimerManager().SetTimer(PoisonUpdateTimerHandle, this, &ABrawlGameMode_Showdown::UpdatePoisonZone, 0.1f, true);
			
			// 설정된 간격으로 데미지 체크
			GetWorld()->GetTimerManager().SetTimer(PoisonDamageTimerHandle, this, &ABrawlGameMode_Showdown::CheckPoisonDamage, PoisonDamageInterval, true);
			
			UE_LOG(LogTemp, Log, TEXT("Poison Cloud Started Shrinking!"));

		}, PoisonStartDelay, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(PoisonUpdateTimerHandle, this, &ABrawlGameMode_Showdown::UpdatePoisonZone, 0.1f, true);
		GetWorld()->GetTimerManager().SetTimer(PoisonDamageTimerHandle, this, &ABrawlGameMode_Showdown::CheckPoisonDamage, PoisonDamageInterval, true);
	}
}

void ABrawlGameMode_Showdown::UpdatePoisonZone()
{
	// 반지름 축소
	float DeltaTime = 0.1f; // 타이머 주기와 일치시켜야 함
	CurrentSafeZoneRadius -= PoisonShrinkSpeed * DeltaTime;

	if (CurrentSafeZoneRadius < MinSafeZoneRadius)
	{
		CurrentSafeZoneRadius = MinSafeZoneRadius;
		// 더 이상 줄어들지 않으면 타이머 멈출 수도 있지만, UI 업데이트 등을 위해 계속 돌릴 수도 있음.
	}

	// 비주얼 업데이트
	if (PoisonZoneInstance)
	{
		PoisonZoneInstance->SetZoneRadius(CurrentSafeZoneRadius);
	}
}

void ABrawlGameMode_Showdown::CheckPoisonDamage()
{
	// 모든 브롤러 검색
	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);

	for (AActor* Actor : FoundBrawlers)
	{
		if (ABrawlCharacter* Brawler = Cast<ABrawlCharacter>(Actor))
		{
			if (Brawler->IsDead()) continue;

			// 맵 중앙(0,0,0) 기준 사각형 범위 체크
			// 원작 브롤스타즈처럼 사각형으로 좁혀오게 하기 위해 X, Y 좌표 각각 비교
			FVector Loc = Brawler->GetActorLocation();
			float DistX = FMath::Abs(Loc.X);
			float DistY = FMath::Abs(Loc.Y);
			
			if (!PoisonDamageEffectClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("BrawlGameMode_Showdown: PoisonDamageEffectClass is null!"));
				return;
			}

			// 안전 구역(Half-Extent) 밖이라면 데미지
			if (DistX > CurrentSafeZoneRadius || DistY > CurrentSafeZoneRadius)
			{
				// GAS를 통한 데미지 적용
				if (UAbilitySystemComponent* ASC = Brawler->GetAbilitySystemComponent())
				{
					FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
					ContextHandle.AddInstigator(PoisonZoneInstance, PoisonZoneInstance); // Instigator를 독구름으로 설정

					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
						PoisonDamageEffectClass, 1.0f, ContextHandle);
					if (SpecHandle.IsValid())
					{
						// SetByCaller로 데미지 값 전달 (Tag: Data.Damage)
						// GE_Poison 블루프린트에서 IncomingDamage의 Magnitude를 SetByCaller(Data.Damage)로 설정해야 함
						SpecHandle.Data.Get()->SetSetByCallerMagnitude(
							FGameplayTag::RequestGameplayTag(FName("Data.Damage")), PoisonDamage);

						ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}
		}
	}
}


AActor* ABrawlGameMode_Showdown::ChoosePlayerStart_Implementation(AController* Player)
{
	// 1. ABrawlSpawnPoint 중에서 Brawler 타입 검색
	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);
	
	TArray<ABrawlSpawnPoint*> ValidSpawnPoints;
	for (AActor* Actor : FoundSpawnPoints)
	{
		if (ABrawlSpawnPoint* SpawnPoint = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SpawnPoint->SpawnPointType == EBrawlSpawnPointType::Brawler)
			{
				ValidSpawnPoints.Add(SpawnPoint);
			}
		}
	}

	if (ValidSpawnPoints.Num() > 0)
	{
		// 랜덤 선택
		int32 RandIndex = FMath::RandRange(0, ValidSpawnPoints.Num() - 1);
		return ValidSpawnPoints[RandIndex];
	}

	// 없다면 기본 로직 (PlayerStart 검색)
	return Super::ChoosePlayerStart(Player);
}

void ABrawlGameMode_Showdown::SpawnBots()
{
	if (AICharacterClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No AI Character Classes set in GameMode!"));
		return;
	}

	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);
	
	// 플레이어 위치 파악 (겹침 방지)
	FVector PlayerLocation = FVector::ZeroVector;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		PlayerLocation = PlayerPawn->GetActorLocation();
	}

	int32 SpawnedBots = 0;
	int32 CurrentTeamID = 2; // 플레이어는 보통 Team 1 또는 0. 봇은 2부터 시작하여 서로 적대적으로 설정.

	// 스폰 포인트 셔플
	int32 LastIndex = FoundSpawnPoints.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = FMath::RandRange(i, LastIndex);
		if (i != Index)
		{
			FoundSpawnPoints.Swap(i, Index);
		}
	}

	for (AActor* Actor : FoundSpawnPoints)
	{
		if (SpawnedBots >= MaxBots) break;

		if (ABrawlSpawnPoint* SpawnPoint = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SpawnPoint->SpawnPointType == EBrawlSpawnPointType::Brawler)
			{
				// 플레이어와 너무 가까우면 스킵 (약 10미터)
				if (FVector::DistSquared(SpawnPoint->GetActorLocation(), PlayerLocation) < 1000000.0f) 
				{
					continue;
				}

				// 랜덤 캐릭터 클래스 선택
				int32 CharIndex = FMath::RandRange(0, AICharacterClasses.Num() - 1);
				TSubclassOf<ABrawlCharacter> BotClass = AICharacterClasses[CharIndex];

				if (BotClass)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
					
					// 봇 스폰
					ABrawlCharacter* NewBot = GetWorld()->SpawnActor<ABrawlCharacter>(BotClass, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation(), SpawnParams);
					
					if (NewBot)
					{
						// AI Controller가 자동으로 생성되겠지만, 팀 ID 설정을 위해 접근
						NewBot->SpawnDefaultController();
						if (AController* BotController = NewBot->GetController())
						{
							// IGenericTeamAgentInterface를 통한 팀 설정
							if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(BotController))
							{
								TeamAgent->SetGenericTeamId(FGenericTeamId(CurrentTeamID));
							}
							else if (IGenericTeamAgentInterface* PawnTeamAgent = Cast<IGenericTeamAgentInterface>(NewBot))
							{
								// 컨트롤러가 안 되면 폰에 직접 설정 시도
								PawnTeamAgent->SetGenericTeamId(FGenericTeamId(CurrentTeamID));
							}

							// AI 설정 (트리 주입)
							ConfigureAI(BotController);
						}

						CurrentTeamID++;
						SpawnedBots++;
					}
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Spawned %d Bots."), SpawnedBots);
}

void ABrawlGameMode_Showdown::ConfigureAI(AController* AIController)
{
	if (ABrawlAIController* BrawlAI = Cast<ABrawlAIController>(AIController))
	{
		if (ShowdownAITree)
		{
			BrawlAI->InjectGameModeSubtree(ShowdownAITree);
		}
	}
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
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Power Cube Box Destroyed. Count not affected."));
	}
}

void ABrawlGameMode_Showdown::DropPowerCubes(AActor* Victim)
{
	if (!Victim || !PowerCubeClass) return;

	int32 DropCount = 1; // 기본 드랍 1개

	// Victim이 가지고 있던 파워 큐브 개수 확인
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Victim))
	{
		bool bFound = false;
		float CurrentCubes = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetPowerCubeCountAttribute(), bFound);
		
		if (bFound && CurrentCubes > 0.0f)
		{
			// 가지고 있던 큐브의 절반을 추가로 드랍 (Brawl Stars 규칙)
			DropCount += FMath::FloorToInt(CurrentCubes / 2.0f);
		}
	}

	FVector CenterLocation = Victim->GetActorLocation();
	
	// 바닥에 붙도록 Z축 조정 (캐릭터 중심이 떠있을 수 있음)
	CenterLocation.Z -= 40.0f; // 대략적인 캡슐 반 높이 고려, 실제로는 Trace를 하는 게 정확하지만 약식으로 처리

	for (int32 i = 0; i < DropCount; i++)
	{
		// 흩뿌리기
		FVector2D RandomOffset = FMath::RandPointInCircle(100.0f);
		FVector SpawnLocation = CenterLocation + FVector(RandomOffset.X, RandomOffset.Y, 0);
		
		// 너무 바닥 아래로 내려가지 않게 보정
		SpawnLocation.Z = FMath::Max(SpawnLocation.Z, 10.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<ABrawlPowerCube>(PowerCubeClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	}

	UE_LOG(LogTemp, Log, TEXT("Dropped %d Power Cubes from %s"), DropCount, *Victim->GetName());
}

void ABrawlGameMode_Showdown::SpawnPowerCubeBoxes()
{
	if (!PowerCubeBoxClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerCubeBoxClass is NOT SET in GameMode!"));
		return;
	}

	int32 SpawnedCount = 0;

	// 1. 우선 ABrawlSpawnPoint 액터를 찾아서 상자 스폰 포인트로 사용
	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlSpawnPoint::StaticClass(), FoundSpawnPoints);

	// 1-1. 스폰 포인트 무작위 셔플
	int32 LastIndex = FoundSpawnPoints.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = FMath::RandRange(i, LastIndex);
		if (i != Index)
		{
			FoundSpawnPoints.Swap(i, Index);
		}
	}

	// 1-2. 상자 스폰
	for (AActor* Actor : FoundSpawnPoints)
	{
		if (ABrawlSpawnPoint* SpawnPoint = Cast<ABrawlSpawnPoint>(Actor))
		{
			if (SpawnPoint->SpawnPointType == EBrawlSpawnPointType::PowerCubeBox)
			{
				if (SpawnedCount >= MaxPowerCubeBoxes) break;

				FVector SpawnLocation = SpawnPoint->GetActorLocation();
				FRotator SpawnRotation = SpawnPoint->GetActorRotation();

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				if (GetWorld()->SpawnActor<AActor>(PowerCubeBoxClass, SpawnLocation, SpawnRotation, SpawnParams))
				{
					SpawnedCount++;
				}
			}
		}
	}

	// 2. 지정된 스폰 포인트가 부족한 경우에만 NavMesh 랜덤 스폰으로 보충 (선택 사항)
	if (SpawnedCount < MaxPowerCubeBoxes)
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSystem)
		{
			int32 RemainingToSpawn = MaxPowerCubeBoxes - SpawnedCount;
			int32 MaxAttempts = RemainingToSpawn * 3;

			for (int32 i = 0; i < MaxAttempts; i++)
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

	UE_LOG(LogTemp, Log, TEXT("Spawned %d PowerCube Boxes (Total)."), SpawnedCount);
}

void ABrawlGameMode_Showdown::CheckGameEndCondition()
{
	// 플레이어가 죽었는지 확인
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	ABrawlCharacter* PlayerCharacter = Cast<ABrawlCharacter>(PlayerPawn);
	
	if (PlayerCharacter && PlayerCharacter->IsDead())
	{
		// 플레이어 사망 -> 게임 종료 (패배)
		// 현재 생존자 수 + 1이 등수
		EndGame(false, AliveBrawlerCount + 1);
		return;
	}

	if (AliveBrawlerCount <= 1)
	{
		// 최후의 1인이 플레이어인지 확인 (위에서 죽었으면 이미 리턴됨)
		if (PlayerCharacter && !PlayerCharacter->IsDead())
		{
			EndGame(true, 1);
		}
		else
		{
			// 플레이어가 없고 AI만 남음 (관전자 모드 등 고려 안함) -> 그냥 종료
			EndGame(false, 1);
		}
	}
}

void ABrawlGameMode_Showdown::EndGame(bool bIsPlayerWinner, int32 PlayerRank)
{
	// 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(PoisonUpdateTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(PoisonDamageTimerHandle);

	if (bIsPlayerWinner)
	{
		UE_LOG(LogTemp, Log, TEXT("GAME OVER! Player WINS! Rank: 1"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GAME OVER! Player LOST! Rank: %d"), PlayerRank);
	}
	
	// 1. GameState 상태 변경 (AI 자동 정지 및 전파)
	if (ABrawlGameState* GS = GetGameState<ABrawlGameState>())
	{
		GS->SetMatchState(EBrawlMatchState::GameOver);
	}

	// 2. 결과 UI 표시 (RPC 호출)
	if (ABrawlStarsTPSPlayerController* PC = Cast<ABrawlStarsTPSPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->ShowMatchResultUI(bIsPlayerWinner, PlayerRank);
	}
}
