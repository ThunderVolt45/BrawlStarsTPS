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

	virtual void PlayerTick(float DeltaTime) override;

public:
	ABrawlStarsTPSPlayerController();

	// 매치 시작 UI 표시
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Brawl|UI")
	void ShowMatchStartUI();

	// 매치 결과 UI 표시
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Brawl|UI")
	void ShowMatchResultUI(bool bIsWinner, int32 Rank);

	/** 현재 조준 중인 타겟 반환 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Aim")
	class ABrawlCharacter* GetCurrentAimTarget() const { return CurrentAimTarget.Get(); }

	/** 조준 보조로 계산된 예측 지점 반환 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Aim")
	FVector GetPredictedAimLocation() const { return PredictedAimLocation; }

protected:
	/** 매치 흐름(Intro/Outro) 제어 컴포넌트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBrawlMatchFlowComponent> MatchFlowComponent;

protected:
	// 조준 보조 감지 반경 (화면 픽셀 단위, 예: 100.0f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Aim")
	float AimDetectionRadius = 100.0f;

	// 조준 보조 회전 속도 (Interpolation Speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Aim")
	float AimAssistInterpSpeed = 5.0f;

	// 현재 조준된 타겟
	TWeakObjectPtr<class ABrawlCharacter> CurrentAimTarget;

	// 조준 보조가 계산한 예측 위치 (발사체 보정용)
	FVector PredictedAimLocation = FVector::ZeroVector;

	// 가장 좋은 타겟 찾기
	void FindBestTarget();

	// 조준 보조 적용 (회전)
	void ApplyAimAssist(float DeltaTime);
};
