// Copyright Epic Games, Inc. All Rights Reserved.

#include "BrawlStarsTPSPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "UI/BrawlHUDWidget.h"
#include "Components/BrawlMatchFlowComponent.h"
#include "AbilitySystemInterface.h"

ABrawlStarsTPSPlayerController::ABrawlStarsTPSPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	// 매치 흐름 컴포넌트 생성
	MatchFlowComponent = CreateDefaultSubobject<UBrawlMatchFlowComponent>(TEXT("MatchFlowComponent"));
}

void ABrawlStarsTPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController()) return;
	
	if (!BrawlHUDClass) return;

	// 위젯이 없다면 새로 생성
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
	if (!IsLocalPlayerController()) return;

	// Add Input Mapping Contexts
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void ABrawlStarsTPSPlayerController::ShowMatchStartUI_Implementation()
{
	if (IsLocalPlayerController() && MatchFlowComponent)
	{
		MatchFlowComponent->StartIntroSequence();
	}
}

void ABrawlStarsTPSPlayerController::ShowMatchResultUI_Implementation(bool bIsWinner, int32 Rank)
{
	UE_LOG(LogTemp, Warning, TEXT("PC: ShowMatchResultUI_Implementation RPC Received! Winner: %d, Rank: %d"), bIsWinner, Rank);

	if (IsLocalPlayerController())
	{
		if (MatchFlowComponent)
		{
			MatchFlowComponent->StartOutroSequence(bIsWinner, Rank);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PC: MatchFlowComponent is NULL on local PC!"));
		}
	}
}