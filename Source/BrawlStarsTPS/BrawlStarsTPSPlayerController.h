// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BrawlStarsTPSPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UBrawlHUDWidget;
class UBrawlMatchFlowComponent;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class BRAWLSTARSTPS_API ABrawlStarsTPSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	/** Main HUD Widget Class */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UBrawlHUDWidget> BrawlHUDClass;

	/** Pointer to the HUD widget */
	UPROPERTY()
	TObjectPtr<UBrawlHUDWidget> BrawlHUDWidget;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Gameplay initialization */
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

public:
	ABrawlStarsTPSPlayerController();

	// 매치 시작 UI 표시
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Brawl|UI")
	void ShowMatchStartUI();

	// 매치 결과 UI 표시
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Brawl|UI")
	void ShowMatchResultUI(bool bIsWinner, int32 Rank);

protected:
	/** 매치 흐름(Intro/Outro) 제어 컴포넌트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBrawlMatchFlowComponent> MatchFlowComponent;
};
