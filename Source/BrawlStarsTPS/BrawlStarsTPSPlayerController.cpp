// Copyright Epic Games, Inc. All Rights Reserved.


#include "BrawlStarsTPSPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "BrawlStarsTPS.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UI/BrawlHUDWidget.h"
#include "UI/BrawlMatchResultWidget.h"
#include "UI/BrawlFinalSummaryWidget.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void ABrawlStarsTPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 오디오 컴포넌트 생성 (여기서 생성하거나 PlayBGM에서 생성)
	if (!BGMComponent)
	{
		BGMComponent = UGameplayStatics::SpawnSound2D(this, nullptr);
	}
	
	if (IsLocalPlayerController() && BrawlHUDClass)
	{
		// 위젯이 이미 있다면 생성하지 않음
		if (!BrawlHUDWidget)
		{
			BrawlHUDWidget = CreateWidget<UBrawlHUDWidget>(this, BrawlHUDClass);
			if (BrawlHUDWidget)
			{
				BrawlHUDWidget->AddToViewport();
			}
		}
		
		// Pawn이 있다면 연결 (이미 BeginPlay 시점에 Pawn이 있을 수 있음)
		if (GetPawn())
		{
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetPawn()))
			{
				if (BrawlHUDWidget)
				{
					BrawlHUDWidget->BindAttributeCallbacks(ASI->GetAbilitySystemComponent());
				}
			}
		}
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogBrawlStarsTPS, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void ABrawlStarsTPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// 서버 로직: 필요한 경우 여기서 처리 (UI는 클라이언트 소관이므로 주로 여기선 스킵)
}

void ABrawlStarsTPSPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	// 클라이언트 로직: 로컬 플레이어가 Pawn을 빙의했을 때 호출됨
	if (IsLocalPlayerController() && BrawlHUDWidget)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(P))
		{
			BrawlHUDWidget->BindAttributeCallbacks(ASI->GetAbilitySystemComponent());
		}
	}
}

void ABrawlStarsTPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool ABrawlStarsTPSPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void ABrawlStarsTPSPlayerController::ShowMatchStartUI()
{
	if (!IsLocalPlayerController()) return;
	
	PlayBGM(MatchStartBGM, 0.5f, 0.0f);
	
	if (MatchStartWidgetClass)
	{
		if (!MatchStartWidget)
		{
			MatchStartWidget = CreateWidget<UUserWidget>(this, MatchStartWidgetClass);
		}
		
		if (MatchStartWidget && !MatchStartWidget->IsInViewport())
		{
			MatchStartWidget->AddToViewport(10);
		}
	}
}

void ABrawlStarsTPSPlayerController::StartGameplayBGM()
{
	PlayBGM(GameplayBGM, 0.5f, 0.5f);
	
	// 게임 시작 UI가 남아있다면 여기서 제거
	if (MatchStartWidget && MatchStartWidget->IsInViewport())
	{
		MatchStartWidget->RemoveFromParent();
	}
}

void ABrawlStarsTPSPlayerController::ShowMatchResultUI(bool bIsWinner, int32 Rank)
{
	if (!IsLocalPlayerController()) return;
	
	// 나중에 사용하기 위해 순위 저장
	SavedRank = Rank;

	PlayBGM(bIsWinner ? WinBGM : LoseBGM, 1.0f, 0.5f);
	
	// HUD 숨김 처리
	if (BrawlHUDWidget)
	{
		BrawlHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (MatchResultWidgetClass)
	{
		if (!MatchResultWidget)
		{
			MatchResultWidget = CreateWidget<UUserWidget>(this, MatchResultWidgetClass);
		}
		
		if (MatchResultWidget)
		{
			// 직접 캐스팅하여 위젯 설정
			if (UBrawlMatchResultWidget* ResultWidget = Cast<UBrawlMatchResultWidget>(MatchResultWidget))
			{
				ResultWidget->SetupResult(bIsWinner, Rank);
				
				// 나가기 버튼 클릭 이벤트 바인딩
				ResultWidget->OnExitClicked.AddDynamic(this, &ABrawlStarsTPSPlayerController::OnMatchExitClicked);
			}

			if (!MatchResultWidget->IsInViewport())
			{
				MatchResultWidget->AddToViewport(20);
			}
			
			// 입력 모드 변경
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(MatchResultWidget->TakeWidget());
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

void ABrawlStarsTPSPlayerController::OnMatchExitClicked()
{
	StartFinalResultSequence();
}

void ABrawlStarsTPSPlayerController::StartFinalResultSequence()
{
	// 1. 첫 번째 결과 위젯(승패 메시지) 제거
	if (MatchResultWidget)
	{
		MatchResultWidget->RemoveFromParent();
		MatchResultWidget = nullptr;
	}

	// 2. 설정된 태그 목록을 순회하며 액터 파괴
	TArray<AActor*> ActorsToDestroy;
	for (const FName& Tag : TagsToDestroy)
	{
		TArray<AActor*> Temp;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, Temp);
		ActorsToDestroy.Append(Temp);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("BrawlStarsTPSPlayerController::StartFinalResultSequence - Found %d Actors to Destroy..."), ActorsToDestroy.Num());
	
	for (AActor* Actor : ActorsToDestroy)
	{
		// 자기 자신(Pawn)은 제외
		if (Actor && Actor != GetPawn())
		{
			Actor->Destroy();
		}
	}

	// 3. 플레이어 브롤러 및 카메라 배치
	TArray<AActor*> Spots;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), EndGameSpotTag, Spots);
	
	if (Spots.Num() > 0)
	{
		AActor* TargetSpot = Spots[0];
		if (APawn* MyPawn = GetPawn())
		{
			MyPawn->SetActorLocationAndRotation(TargetSpot->GetActorLocation(), TargetSpot->GetActorRotation());
			MyPawn->DisableInput(this);
		}
		
		// 카메라 전환 (연출용 카메라가 있다면)
		SetViewTargetWithBlend(TargetSpot, 0.5f);
	}

	// 4. 최종 요약 위젯 표시
	if (FinalSummaryWidgetClass)
	{
		if (!FinalSummaryWidget)
		{
			FinalSummaryWidget = CreateWidget<UUserWidget>(this, FinalSummaryWidgetClass);
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

			// 입력 모드 유지 및 포커스 전환
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(FinalSummaryWidget->TakeWidget());
			SetInputMode(InputMode);
		}
	}
}

void ABrawlStarsTPSPlayerController::PlayBGM(USoundBase* NewBGM, float FadeOutDuration, float FadeInDuration)
{
	if (!NewBGM) return;
	
	if (BGMComponent && BGMComponent->IsPlaying())
	{
		if (BGMComponent->Sound == NewBGM) return;

		BGMComponent->FadeOut(FadeOutDuration, 0.0f);
	}
	
	BGMComponent = UGameplayStatics::SpawnSound2D(this, NewBGM);

	if (BGMComponent)
	{
		BGMComponent->FadeIn(FadeInDuration);
	}
}