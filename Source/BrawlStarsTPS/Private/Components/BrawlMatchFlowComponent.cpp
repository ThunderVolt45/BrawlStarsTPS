// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BrawlMatchFlowComponent.h"

#include "BrawlCharacter.h"
#include "BrawlStarsTPSPlayerController.h"
#include "BrawlGameState_Knockout.h"
#include "Blueprint/UserWidget.h"
#include "UI/BrawlMatchStartWidget.h"
#include "UI/BrawlMatchResultWidget.h"
#include "UI/BrawlRoundResultWidget.h"
#include "UI/BrawlFinalSummaryWidget.h"
#include "UI/BrawlHUDWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraActor.h"
#include "TimerManager.h"

UBrawlMatchFlowComponent::UBrawlMatchFlowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 기본 파괴 태그 설정
	TagsToDestroy.Add(FName("Destructible"));
	TagsToDestroy.Add(FName("Brawler"));
	TagsToDestroy.Add(FName("Box"));
	TagsToDestroy.Add(FName("Cube"));
}

void UBrawlMatchFlowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsOrbiting && OrbitCameraActor)
	{
		FRotator NewRotation = OrbitCameraActor->GetActorRotation();
		NewRotation.Yaw += OrbitSpeed * DeltaTime;
		OrbitCameraActor->SetActorRotation(NewRotation);
	}
}

void UBrawlMatchFlowComponent::BeginPlay()
{
	Super::BeginPlay();

	// 소유자는 반드시 PlayerController여야 함
	APlayerController* PC = CastChecked<APlayerController>(GetOwner());

	if (PC->IsLocalPlayerController())
	{
		// 오디오 컴포넌트 생성 및 등록 (실패 시 즉시 감지)
		if (!BGMComponent)
		{
			BGMComponent = NewObject<UAudioComponent>(GetOwner(), UAudioComponent::StaticClass());
			check(BGMComponent);
			BGMComponent->RegisterComponent();
			BGMComponent->bAutoDestroy = false;
			BGMComponent->bStopWhenOwnerDestroyed = true;
		}

		ABrawlGameState* GS = GetWorld()->GetGameState<ABrawlGameState>();
		check(GS); // GameState는 반드시 존재해야 함

		GS->OnMatchStateChanged.AddDynamic(this, &UBrawlMatchFlowComponent::OnMatchStateChanged);
		
		// 초기 상태 체크
		if (GS->GetMatchState() != EBrawlMatchState::Waiting)
		{
			OnMatchStateChanged();
		}
	}
}

void UBrawlMatchFlowComponent::OnMatchStateChanged()
{
	ABrawlGameState* GS = GetWorld()->GetGameState<ABrawlGameState>();
	check(GS);

	OwnerController = CastChecked<ABrawlStarsTPSPlayerController>(GetOwner());
	if (!OwnerController->IsLocalPlayerController()) return;

	UE_LOG(LogTemp, Warning, TEXT("MatchFlow: OnMatchStateChanged called. NewState: %d"), (uint8)GS->GetMatchState());

	switch (GS->GetMatchState())
	{
	case EBrawlMatchState::Intro:
		HandleIntroStarted();
		break;
	case EBrawlMatchState::MatchStart:
		HandleMatchStartStarted();
		break;
	case EBrawlMatchState::Playing:
		HandlePlayingStarted();
		break;
	case EBrawlMatchState::Intermission:
		HandleIntermissionStarted();
		break;
	case EBrawlMatchState::GameOver:
		UE_LOG(LogTemp, Warning, TEXT("MatchFlow: GameOver state detected. Waiting for RPC..."));
		break;
	}
}

void UBrawlMatchFlowComponent::HandleMatchStartStarted()
{
	UE_LOG(LogTemp, Warning, TEXT("MatchFlow: HandleMatchStartStarted. START Animation!"));

	bIsOrbiting = false;
	OwnerController = CastChecked<ABrawlStarsTPSPlayerController>(GetOwner());
	
	// 폰으로 시점 복귀
	OwnerController->SetViewTargetWithBlend(OwnerController->GetPawn(), 1.0f);

	if (UBrawlMatchStartWidget* StartWidget = Cast<UBrawlMatchStartWidget>(MatchStartWidget))
	{
		StartWidget->HideInfoText();
		StartWidget->PlayStartAnimation();
	}

	// "START" 사운드 재생
	PlayBGM(MatchStartBGM, 0.1f, 0.0f);
}

void UBrawlMatchFlowComponent::HandlePlayingStarted()
{
	UE_LOG(LogTemp, Warning, TEXT("MatchFlow: HandlePlayingStarted. Enabling Input!"));

	OwnerController = CastChecked<ABrawlStarsTPSPlayerController>(GetOwner());

	// 실제 게임플레이 BGM으로 전환
	PlayBGM(GameplayBGM, 0.0f, 0.0f);

	if (MatchStartWidget)
	{
		MatchStartWidget->RemoveFromParent();
		MatchStartWidget = nullptr;
	}

	// 입력 활성화
	if (OwnerController)
	{
		if (APawn* MyPawn = OwnerController->GetPawn())
		{
			MyPawn->EnableInput(OwnerController);
		}
		
		FInputModeGameOnly InputMode;
		OwnerController->SetInputMode(InputMode);
		OwnerController->bShowMouseCursor = false;
	}
}

void UBrawlMatchFlowComponent::HandleIntermissionStarted()
{
	OwnerController = CastChecked<ABrawlStarsTPSPlayerController>(GetOwner());
	if (!OwnerController->IsLocalPlayerController()) return;

	// 조작 차단 및 UI 모드 전환
	if (APawn* MyPawn = OwnerController->GetPawn())
	{
		MyPawn->DisableInput(OwnerController);
	}
	OwnerController->SetInputMode(FInputModeUIOnly());
	OwnerController->bShowMouseCursor = true;

	ABrawlGameState_Knockout* KGS = GetWorld()->GetGameState<ABrawlGameState_Knockout>();
	if (!KGS)
	{
		UE_LOG(LogTemp, Error, TEXT("MatchFlow: FAILED to get BrawlGameState_Knockout on Client!"));
		return;
	}

	int32 Winner = KGS->GetLastRoundWinner();
	int32 T0Wins = KGS->GetTeam0Wins();
	int32 T1Wins = KGS->GetTeam1Wins();

	UE_LOG(LogTemp, Log, TEXT("MatchFlow: Intermission Data - Winner: %d, T0: %d, T1: %d"), Winner, T0Wins, T1Wins);

	// 플레이어가 승리 팀인지 확인
	bool bIsPlayerWinner = false;
	if (ABrawlCharacter* MyChar = Cast<ABrawlCharacter>(OwnerController->GetPawn()))
	{
		if (MyChar->GetTeamID() == Winner) bIsPlayerWinner = true;
	}

	StartRoundResultSequence(bIsPlayerWinner, T0Wins, T1Wins);
}

void UBrawlMatchFlowComponent::StartIntroSequence()
{
	// GameState의 상태 변화로 자동 처리됨
}

void UBrawlMatchFlowComponent::HandleIntroStarted()
{
	// 준비 BGM 즉시 재생
	PlayBGM(MatchReadyBGM, 0.5f, 0.0f);

	if (MatchStartWidgetClass)
	{
		if (!MatchStartWidget)
		{
			MatchStartWidget = CreateWidget<UUserWidget>(OwnerController, MatchStartWidgetClass);
		}

		if (MatchStartWidget)
		{
			if (UBrawlMatchStartWidget* StartWidget = Cast<UBrawlMatchStartWidget>(MatchStartWidget))
			{
				FText ModeName = FText::FromString(TEXT("GAME MODE"));
				FText ModeDesc = FText::FromString(TEXT("DEFEAT ENEMIES"));

				if (ABrawlGameState* GS = GetWorld()->GetGameState<ABrawlGameState>())
				{
					ModeName = GS->GetModeName();
					ModeDesc = GS->GetModeDescription();
				}

				StartWidget->SetupMatchInfo(ModeName, ModeDesc);
			}

			if (!MatchStartWidget->IsInViewport())
			{
				MatchStartWidget->AddToViewport(10);
			}
		}
	}

	// 조작 차단 유지 및 UI 모드
	if (OwnerController)
	{
		if (APawn* MyPawn = OwnerController->GetPawn())
		{
			MyPawn->DisableInput(OwnerController);
		}
		OwnerController->SetInputMode(FInputModeUIOnly());
		OwnerController->bShowMouseCursor = true;
	}

	// 2초 뒤 궤도 카메라 시작
	GetWorld()->GetTimerManager().SetTimer(SequenceTimerHandle, [this]() {
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), OrbitCameraTag, FoundActors);

		if (FoundActors.Num() > 0)
		{
			OrbitCameraActor = FoundActors[0];
			OwnerController->SetViewTargetWithBlend(OrbitCameraActor, 1.0f);
			bIsOrbiting = true;
		}
	}, 2.0f, false);
}

void UBrawlMatchFlowComponent::StartOutroSequence(bool bIsWinner, int32 Rank)
{
	// 소유자 검사 (절대 NULL일 수 없으며 우리 클래스여야 함)
	OwnerController = CastChecked<ABrawlStarsTPSPlayerController>(GetOwner());

	if (!OwnerController->IsLocalPlayerController()) return;

	UE_LOG(LogTemp, Warning, TEXT("MatchFlow: StartOutroSequence PROCEEDING. Winner: %d, Rank: %d"), bIsWinner, Rank);

	SavedRank = Rank;

	// 1. BGM 재생
	PlayBGM(bIsWinner ? WinBGM : LoseBGM, 0.0f, 0.0f);

	if (APawn* MyPawn = OwnerController->GetPawn())
	{
		MyPawn->DisableInput(OwnerController);
	}
	
	// 위젯 클래스 검사 (블루프린트 설정 누락 방지)
	checkf(MatchResultWidgetClass, TEXT("MatchResultWidgetClass is NOT set in MatchFlowComponent! Set it in BP."));

	UE_LOG(LogTemp, Log, TEXT("MatchFlow: Creating Widget with class: %s"), *MatchResultWidgetClass->GetName());
	
	if (!MatchResultWidget)
	{
		MatchResultWidget = CreateWidget<UUserWidget>(OwnerController, MatchResultWidgetClass);
	}
	
	if (MatchResultWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("MatchFlow: Widget Created Successfully."));

		if (UBrawlMatchResultWidget* ResultWidget = Cast<UBrawlMatchResultWidget>(MatchResultWidget))
		{
			ResultWidget->SetupResult(bIsWinner, Rank);
			if (!ResultWidget->OnExitClicked.IsBound())
			{
				ResultWidget->OnExitClicked.AddDynamic(this, &UBrawlMatchFlowComponent::HandleMatchExitClicked);
			}
		}

		if (!MatchResultWidget->IsInViewport())
		{
			MatchResultWidget->AddToViewport(20);
			UE_LOG(LogTemp, Log, TEXT("MatchFlow: Widget Added to Viewport."));
		}
		
		// 입력 모드 전환
		FInputModeUIOnly InputMode;
		OwnerController->SetInputMode(InputMode);
		OwnerController->bShowMouseCursor = true;
	}
	else
	{
		// 위젯 인스턴스 생성 실패 시 크래시 (시스템상 발생하면 안 되는 상황)
		checkf(false, TEXT("MatchFlow: FAILED to create widget instance from class %s"), *MatchResultWidgetClass->GetName());
	}
}

void UBrawlMatchFlowComponent::StartRoundResultSequence(bool bIsWinner, int32 Team1Score, int32 Team2Score)
{
	OwnerController = CastChecked<ABrawlStarsTPSPlayerController>(GetOwner());
	if (!OwnerController->IsLocalPlayerController()) return;

	UE_LOG(LogTemp, Warning, TEXT("MatchFlow: StartRoundResultSequence. Winner: %d, Score: %d-%d"), bIsWinner, Team1Score, Team2Score);

	if (RoundResultWidgetClass)
	{
		UUserWidget* RoundWidget = CreateWidget<UUserWidget>(OwnerController, RoundResultWidgetClass);
		if (RoundWidget)
		{
			UE_LOG(LogTemp, Log, TEXT("MatchFlow: RoundResultWidget Created."));
			if (UBrawlRoundResultWidget* ResultWidget = Cast<UBrawlRoundResultWidget>(RoundWidget))
			{
				ResultWidget->SetupRoundResult(bIsWinner, Team1Score, Team2Score);
			}

			RoundWidget->AddToViewport(15);
			
			// 일정 시간 후 위젯 제거 (3초 뒤 제거)
			FTimerHandle WidgetTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(WidgetTimerHandle, [RoundWidget]()
			{
				if (RoundWidget)
				{
					RoundWidget->RemoveFromParent();
					UE_LOG(LogTemp, Log, TEXT("MatchFlow: RoundResultWidget Removed."));
				}
			}, 3.0f, false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MatchFlow: FAILED to create RoundResultWidget instance from Class: %s"), *RoundResultWidgetClass->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MatchFlow: RoundResultWidgetClass is NULL! Check BP_BrawlMatchFlowComponent."));
	}
}

void UBrawlMatchFlowComponent::PlayBGM(USoundBase* NewBGM, float FadeOutDuration, float FadeInDuration)
{
	if (!NewBGM || !BGMComponent) return;
	
	// 이미 같은 사운드가 재생 중이면 무시
	if (BGMComponent->IsPlaying() && BGMComponent->Sound == NewBGM) return;

	// 기존 사운드 페이드 아웃
	if (BGMComponent->IsPlaying())
	{
		BGMComponent->FadeOut(FadeOutDuration, 0.0f);
	}

	// 새 사운드 설정 및 페이드 인
	BGMComponent->SetSound(NewBGM);
	BGMComponent->FadeIn(FadeInDuration, 1.0f);
}

void UBrawlMatchFlowComponent::HandleMatchExitClicked()
{
	Outro_StartFinalSummary();
}

void UBrawlMatchFlowComponent::Outro_StartFinalSummary()
{
	if (!OwnerController) return;

	if (MatchResultWidget)
	{
		MatchResultWidget->RemoveFromParent();
		MatchResultWidget = nullptr;
	}
	
	/*
	// 레벨 정리
	TArray<AActor*> ActorsToDestroy;
	for (const FName& Tag : TagsToDestroy)
	{
		TArray<AActor*> FoundedActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundedActors);
		ActorsToDestroy.Append(FoundedActors);
	}
	
	for (AActor* Actor : ActorsToDestroy)
	{
		if (Actor && Actor != OwnerController->GetPawn())
		{
			Actor->Destroy();
		}
	}

	// 배치
	TArray<AActor*> Spots;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), EndGameSpotTag, Spots);
	
	if (Spots.Num() > 0)
	{
		AActor* TargetSpot = Spots[0];
		if (APawn* MyPawn = OwnerController->GetPawn())
		{
			MyPawn->SetActorLocationAndRotation(TargetSpot->GetActorLocation(), TargetSpot->GetActorRotation());
			MyPawn->DisableInput(OwnerController);
		}
		OwnerController->SetViewTargetWithBlend(TargetSpot, 0.5f);
	}

	// 최종 요약 위젯
	if (FinalSummaryWidgetClass)
	{
		if (!FinalSummaryWidget)
		{
			FinalSummaryWidget = CreateWidget<UUserWidget>(OwnerController, FinalSummaryWidgetClass);
		}

		if (FinalSummaryWidget)
		{
			if (UBrawlFinalSummaryWidget* SummaryWidget = Cast<UBrawlFinalSummaryWidget>(FinalSummaryWidget))
			{
				SummaryWidget->SetupFinalSummary(SavedRank);
			}

			if (!FinalSummaryWidget->IsInViewport())
			{
				FinalSummaryWidget->AddToViewport(30);
			}

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(FinalSummaryWidget->TakeWidget());
			OwnerController->SetInputMode(InputMode);
		}
	}
	*/

	UGameplayStatics::OpenLevel(this, FName("LV_Lobby"));
}
