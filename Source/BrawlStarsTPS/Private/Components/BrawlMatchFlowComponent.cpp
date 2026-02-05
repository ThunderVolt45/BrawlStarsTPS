// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BrawlMatchFlowComponent.h"
#include "BrawlStarsTPSPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/BrawlMatchStartWidget.h"
#include "UI/BrawlMatchResultWidget.h"
#include "UI/BrawlFinalSummaryWidget.h"
#include "UI/BrawlHUDWidget.h"
#include "Kismet/GameplayStatics.h"
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

void UBrawlMatchFlowComponent::StartIntroSequence()
{
	OwnerController = Cast<ABrawlStarsTPSPlayerController>(GetOwner());
	IntroStep1_ShowInfo();
}

void UBrawlMatchFlowComponent::StartOutroSequence(bool bIsWinner, int32 Rank)
{
	OwnerController = Cast<ABrawlStarsTPSPlayerController>(GetOwner());
	if (!OwnerController) return;

	SavedRank = Rank;

	// BGM 재생 및 HUD 숨김
	OwnerController->PlayBGM(bIsWinner ? WinBGM : LoseBGM, 1.0f, 0.5f);
	
	// HUD 위젯 접근을 위해 Controller의 멤버에 접근 (필요 시 HUD 관련 로직도 이관 가능)
	// 여기서는 일단 직접 캐스팅하여 처리
	if (OwnerController->GetHUD()) 
	{
		// ...
	}

	if (MatchResultWidgetClass)
	{
		if (!MatchResultWidget)
		{
			MatchResultWidget = CreateWidget<UUserWidget>(OwnerController, MatchResultWidgetClass);
		}
		
		if (MatchResultWidget)
		{
			if (UBrawlMatchResultWidget* ResultWidget = Cast<UBrawlMatchResultWidget>(MatchResultWidget))
			{
				ResultWidget->SetupResult(bIsWinner, Rank);
				ResultWidget->OnExitClicked.AddDynamic(this, &UBrawlMatchFlowComponent::HandleMatchExitClicked);
			}

			if (!MatchResultWidget->IsInViewport())
			{
				MatchResultWidget->AddToViewport(20);
			}
			
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(MatchResultWidget->TakeWidget());
			OwnerController->SetInputMode(InputMode);
			OwnerController->bShowMouseCursor = true;
		}
	}
}

void UBrawlMatchFlowComponent::HandleMatchExitClicked()
{
	Outro_StartFinalSummary();
}

void UBrawlMatchFlowComponent::IntroStep1_ShowInfo()
{
	if (!OwnerController) return;

	OwnerController->PlayBGM(MatchStartBGM, 0.5f, 0.0f);

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
				StartWidget->SetupMatchInfo(FText::FromString(TEXT("SHOWDOWN")), FText::FromString(TEXT("DEFEAT ALL OTHER BRAWLERS")));
			}

			if (!MatchStartWidget->IsInViewport())
			{
				MatchStartWidget->AddToViewport(10);
			}
		}
	}

	if (APawn* MyPawn = OwnerController->GetPawn())
	{
		MyPawn->DisableInput(OwnerController);
	}

	GetWorld()->GetTimerManager().SetTimer(SequenceTimerHandle, this, &UBrawlMatchFlowComponent::IntroStep2_OrbitCamera, 2.0f, false);
}

void UBrawlMatchFlowComponent::IntroStep2_OrbitCamera()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), OrbitCameraTag, FoundActors);

	if (FoundActors.Num() > 0)
	{
		OrbitCameraActor = FoundActors[0];
		OwnerController->SetViewTargetWithBlend(OrbitCameraActor, 1.0f);
		bIsOrbiting = true;
	}

	GetWorld()->GetTimerManager().SetTimer(SequenceTimerHandle, this, &UBrawlMatchFlowComponent::IntroStep3_ReturnToPawn, 3.0f, false);
}

void UBrawlMatchFlowComponent::IntroStep3_ReturnToPawn()
{
	bIsOrbiting = false;
	if (OwnerController)
	{
		OwnerController->SetViewTargetWithBlend(OwnerController->GetPawn(), 1.0f);
	}

	if (UBrawlMatchStartWidget* StartWidget = Cast<UBrawlMatchStartWidget>(MatchStartWidget))
	{
		StartWidget->HideInfoText();
	}

	GetWorld()->GetTimerManager().SetTimer(SequenceTimerHandle, this, &UBrawlMatchFlowComponent::IntroStep4_ShowStartLogo, 1.0f, false);
}

void UBrawlMatchFlowComponent::IntroStep4_ShowStartLogo()
{
	if (UBrawlMatchStartWidget* StartWidget = Cast<UBrawlMatchStartWidget>(MatchStartWidget))
	{
		StartWidget->PlayStartAnimation();
	}

	if (OwnerController) OwnerController->StartGameplayBGM();

	GetWorld()->GetTimerManager().SetTimer(SequenceTimerHandle, this, &UBrawlMatchFlowComponent::IntroStep5_BeginPlay, 1.5f, false);
}

void UBrawlMatchFlowComponent::IntroStep5_BeginPlay()
{
	if (MatchStartWidget)
	{
		MatchStartWidget->RemoveFromParent();
		MatchStartWidget = nullptr;
	}

	if (OwnerController && OwnerController->GetPawn())
	{
		OwnerController->GetPawn()->EnableInput(OwnerController);
		
		FInputModeGameOnly InputMode;
		OwnerController->SetInputMode(InputMode);
		OwnerController->bShowMouseCursor = false;
	}
}

void UBrawlMatchFlowComponent::Outro_StartFinalSummary()
{
	if (!OwnerController) return;

	if (MatchResultWidget)
	{
		MatchResultWidget->RemoveFromParent();
		MatchResultWidget = nullptr;
	}

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
}
