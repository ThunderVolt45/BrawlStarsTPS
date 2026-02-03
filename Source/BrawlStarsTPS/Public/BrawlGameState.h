// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BrawlGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBrawlerKilledDelegate, AActor*, Killer, AActor*, Victim);

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

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 현재 생존한 브롤러 수 (Replicated)
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Brawl|Game")
	int32 AliveBrawlerCount = 0;
};
