// Copyright Epic Games, Inc. All Rights Reserved.


#include "BrawlStarsTPSPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "BrawlStarsTPS.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UI/BrawlHUDWidget.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

void ABrawlStarsTPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

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