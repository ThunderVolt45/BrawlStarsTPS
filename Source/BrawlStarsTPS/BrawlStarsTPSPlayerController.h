// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BrawlStarsTPSPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UBrawlHUDWidget;
class UBrawlMatchResultWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class ABrawlStarsTPSPlayerController : public APlayerController
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

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:
	// 매치 시작 UI 표시
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void ShowMatchStartUI();

	// 매치 결과 UI 표시
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void ShowMatchResultUI(bool bIsWinner, int32 Rank);

	// BGM 재생 (기존 BGM은 FadeOut)
	UFUNCTION(BlueprintCallable, Category = "Brawl|Audio")
	void PlayBGM(USoundBase* NewBGM, float FadeOutDuration = 1.0f, float FadeInDuration = 1.0f);

	// 게임플레이(전투) BGM 시작 (GameMode에서 호출)
	void StartGameplayBGM();

protected:
	// 매치 시작(카운트다운) BGM
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Audio")
	TObjectPtr<USoundBase> MatchStartBGM;

	// 게임 진행 중 BGM
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Audio")
	TObjectPtr<USoundBase> GameplayBGM;

	// 승리 시 BGM
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Audio")
	TObjectPtr<USoundBase> WinBGM;

	// 패배 시 BGM
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Audio")
	TObjectPtr<USoundBase> LoseBGM;
	
	// BGM 재생용 컴포넌트
	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMComponent;

	// 매치 시작 위젯 클래스 (카운트다운)
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> MatchStartWidgetClass;

	// 매치 결과 위젯 클래스 (승리/패배)
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> MatchResultWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MatchStartWidget;

	UPROPERTY()
	TObjectPtr<UBrawlMatchResultWidget> MatchResultWidget;
};
