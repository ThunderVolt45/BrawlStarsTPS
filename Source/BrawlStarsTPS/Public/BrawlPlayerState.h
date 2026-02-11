// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GenericTeamAgentInterface.h"
#include "BrawlPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBountyChanged, int32, NewBounty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTieBreakerStateChanged, bool, bHasTieBreaker);

/**
 * ABrawlPlayerState
 * 
 * 브롤러의 개별 상태(팀, 현상금, 점수 기여도 등)를 관리하는 클래스
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlPlayerState : public APlayerState, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ABrawlPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IGenericTeamAgentInterface 구현
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	/** 현상금 설정 (서버 전용) */
	void SetBounty(int32 NewBounty);

	/** 현상금 추가 (최대 7점) */
	void AddBounty(int32 Amount);

	/** 현상금 초기화 (사망/시작 시 호출) */
	void ResetBounty();

	UFUNCTION(BlueprintCallable, Category = "Brawl|PlayerState")
	int32 GetBounty() const { return CurrentBounty; }

	/** 팀 점수 기여도 추가 */
	void AddScoreContribution(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Brawl|PlayerState")
	int32 GetScoreContribution() const { return ScoreContribution; }

	/** 팀 ID 설정 (0 또는 1) */
	void SetTeamID(int32 NewTeamID);

	UFUNCTION(BlueprintCallable, Category = "Brawl|PlayerState")
	int32 GetTeamID() const { return (int32)TeamID.GetId(); }

	/** 타이 브레이커 아이템 보유 여부 설정 */
	void SetHasTieBreaker(bool bHas);

	UFUNCTION(BlueprintCallable, Category = "Brawl|PlayerState")
	bool HasTieBreaker() const { return bHasTieBreaker; }

public:
	// 현상금 변경 델리게이트 (UI 연동용)
	UPROPERTY(BlueprintAssignable, Category = "Brawl|PlayerState")
	FOnBountyChanged OnBountyChanged;

	// 타이 브레이커 상태 변경 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Brawl|PlayerState")
	FOnTieBreakerStateChanged OnTieBreakerStateChanged;

protected:
	UFUNCTION()
	void OnRep_CurrentBounty();

	UFUNCTION()
	void OnRep_TeamID();

	UFUNCTION()
	void OnRep_HasTieBreaker();

protected:
	// 현재 현상금 (2~7점)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentBounty, VisibleInstanceOnly, Category = "Brawl|PlayerState")
	int32 CurrentBounty = 2;

	// 팀 점수 기여도
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|PlayerState")
	int32 ScoreContribution = 0;

	// 팀 ID
	UPROPERTY(ReplicatedUsing = OnRep_TeamID, VisibleInstanceOnly, Category = "Brawl|PlayerState")
	FGenericTeamId TeamID = FGenericTeamId::NoTeam;

	// 타이 브레이커 보유 여부
	UPROPERTY(ReplicatedUsing = OnRep_HasTieBreaker, VisibleInstanceOnly, Category = "Brawl|PlayerState")
	bool bHasTieBreaker = false;
};
