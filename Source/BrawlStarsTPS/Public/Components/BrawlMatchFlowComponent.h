// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BrawlMatchFlowComponent.generated.h"

class ABrawlStarsTPSPlayerController;
class UUserWidget;
class USoundBase;

/**
 * UBrawlMatchFlowComponent
 * 
 * 게임의 시작(Intro)과 종료(Outro) 시퀀스 연출 및 UI 흐름을 전담하는 컴포넌트입니다.
 * PlayerController의 비대한 로직을 분리하기 위해 생성되었습니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BRAWLSTARSTPS_API UBrawlMatchFlowComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBrawlMatchFlowComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 게임 시작 연출 시작 */
	void StartIntroSequence();

	/** 게임 종료 연출 시작 */
	void StartOutroSequence(bool bIsWinner, int32 Rank);

	/** 매치 결과 위젯의 나가기 클릭 시 호출 */
	UFUNCTION()
	void HandleMatchExitClicked();

	/** BGM 재생 (기존 BGM은 FadeOut) */
	void PlayBGM(class USoundBase* NewBGM, float FadeOutDuration = 1.0f, float FadeInDuration = 1.0f);

protected:
	/** 상태 변경 이벤트 핸들러 */
	UFUNCTION()
	void OnMatchStateChanged();

	/** 각 상태별 연출 시작 함수 */
	void HandleIntroStarted();
	void HandlePlayingStarted();

	/** 레벨 정리 및 최종 결과 표시 */
	void Outro_StartFinalSummary();

public:
	/** 설정값들 (BP에서 설정 가능) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|UI")
	TSubclassOf<UUserWidget> MatchStartWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|UI")
	TSubclassOf<UUserWidget> MatchResultWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|UI")
	TSubclassOf<UUserWidget> FinalSummaryWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|Audio")
	TObjectPtr<USoundBase> MatchStartBGM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|Audio")
	TObjectPtr<USoundBase> GameplayBGM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|Audio")
	TObjectPtr<USoundBase> WinBGM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|Audio")
	TObjectPtr<USoundBase> LoseBGM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|Camera")
	FName OrbitCameraTag = FName("OrbitCamera");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|Camera")
	float OrbitSpeed = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|Cleanup")
	TArray<FName> TagsToDestroy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Brawl|Flow|Cleanup")
	FName EndGameSpotTag = FName("EndGameSpot");

private:
	/** 내부 상태 및 관리 변수 */
	UPROPERTY()
	TObjectPtr<ABrawlStarsTPSPlayerController> OwnerController;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMComponent;

	UPROPERTY()
	TObjectPtr<UUserWidget> MatchStartWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> MatchResultWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> FinalSummaryWidget;

	FTimerHandle SequenceTimerHandle;
	bool bIsOrbiting = false;
	int32 SavedRank = 0;

	UPROPERTY()
	TObjectPtr<AActor> OrbitCameraActor;
};
