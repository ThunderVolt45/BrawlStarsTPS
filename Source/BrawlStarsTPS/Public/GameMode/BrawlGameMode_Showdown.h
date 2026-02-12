// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlStarsTPSGameMode.h"
#include "BrawlGameMode_Showdown.generated.h"

class ABrawlPowerCubeBox;
class ABrawlPowerCube;

/**
 * ABrawlGameMode_Showdown
 * 
 * 쇼다운 모드 (Battle Royale) 게임 모드
 * - 파워 큐브 상자 생성
 * - 최후의 1인 승리 조건 체크
 * - 독구름 (추후 구현)
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlGameMode_Showdown : public ABrawlStarsTPSGameMode
{
	GENERATED_BODY()
	
public:
	ABrawlGameMode_Showdown();

	virtual void BeginPlay() override;

	// 처치 발생 시 호출 (부모 클래스 오버라이드)
	virtual void NotifyKill(AActor* Killer, AActor* Victim) override;

	/** 게임 시작 실행 */
	virtual void StartMatch() override;

	// 현재 생존 브롤러 수 반환
	UFUNCTION(BlueprintCallable, Category = "Brawl|Showdown")
	int32 GetAliveBrawlerCount() const { return AliveBrawlerCount; }

protected:
	// 파워 큐브 상자 스폰 로직
	void SpawnPowerCubeBoxes();

	// 브롤러 사망 시 파워 큐브 드랍
	void DropPowerCubes(AActor* Victim);

	// 남은 플레이어 수 확인 및 게임 종료 체크
	void CheckGameEndCondition();

	// 팀 및 봇 설정 오버라이드
	virtual void SetupTeams() override;
	virtual void SpawnBots() override;

protected:
	// 스폰할 상자 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown")
	TSubclassOf<AActor> PowerCubeBoxClass;

	// 드랍할 파워 큐브 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown")
	TSubclassOf<AActor> PowerCubeClass;

	// 상자 스폰 개수
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown")
	int32 MaxPowerCubeBoxes = 10;

	// 현재 생존한 브롤러 수 (플레이어 + AI)
	UPROPERTY(VisibleInstanceOnly, Category = "Brawl|Showdown")
	int32 AliveBrawlerCount = 0;
};
