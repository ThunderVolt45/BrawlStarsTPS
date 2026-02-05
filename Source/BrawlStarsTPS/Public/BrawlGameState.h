// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BrawlGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBrawlerKilledDelegate, AActor*, Killer, AActor*, Victim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchStateChanged);

UENUM(BlueprintType)
enum class EBrawlMatchState : uint8
{
	Waiting,    // 대기 중
	Intro,      // 카메라 연출 및 정보 표시
	Playing,    // 실제 게임 진행 중 (조작 허용)
	GameOver    // 종료 및 결과 화면
};

/**
 * 
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	// 블루프린트 UI(Kill Feed 등)에서 바인딩할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Brawl|Game")
	FOnBrawlerKilledDelegate OnBrawlerKilled;

	// 매치 상태 변경 시 호출되는 델리게이트 (연출용)
	UPROPERTY(BlueprintAssignable, Category = "Brawl|Game")
	FOnMatchStateChanged OnMatchStateChanged;

	/** 서버에서 GameMode가 호출하는 함수 */
	void NotifyBrawlerKilled(AActor* Killer, AActor* Victim);

	/** 모든 클라이언트에게 처치 사실을 전파하는 멀티캐스트 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnBrawlerKilled(AActor* Killer, AActor* Victim);

	// 생존자 수 설정 (서버만 호출)
	void SetAliveBrawlerCount(int32 Count);

	// 생존자 수 반환
	UFUNCTION(BlueprintCallable, Category = "Brawl|Game")
	int32 GetAliveBrawlerCount() const { return AliveBrawlerCount; }

	// 매치 상태 설정 (서버 전용)
	void SetMatchState(EBrawlMatchState NewState);

	UFUNCTION(BlueprintCallable, Category = "Brawl|Game")
	EBrawlMatchState GetMatchState() const { return MatchState; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Game")
	bool IsMatchInProgress() const { return MatchState == EBrawlMatchState::Playing; }

	/** 모든 AI 브롤러 활성화/비활성화 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Game")
	void SetAllAIActive(bool bActive);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 상태 변경 리플리케이트 콜백
	UFUNCTION()
	void OnRep_MatchState();

protected:
	// 현재 생존한 브롤러 수 (Replicated)
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|Game")
	int32 AliveBrawlerCount = 0;

	// 현재 매치 상태 (Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_MatchState, VisibleInstanceOnly, Category = "Brawl|Game")
	EBrawlMatchState MatchState = EBrawlMatchState::Waiting;

private:
	/** 레벨 내의 모든 AI 컨트롤러 캐싱 */
	UPROPERTY()
	TArray<TObjectPtr<class ABrawlAIController>> CachedAIControllers;
};
