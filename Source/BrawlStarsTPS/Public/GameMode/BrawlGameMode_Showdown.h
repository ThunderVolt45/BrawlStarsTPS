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

	// 플레이어 시작 지점 선택 (부모 클래스 오버라이드)
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 처치 발생 시 호출 (부모 클래스 오버라이드)
	virtual void NotifyKill(AActor* Killer, AActor* Victim) override;

protected:
	// 파워 큐브 상자 스폰 로직
	void SpawnPowerCubeBoxes();

	// AI 봇 스폰 로직
	void SpawnBots();

	// 브롤러 사망 시 파워 큐브 드랍
	void DropPowerCubes(AActor* Victim);

	// 남은 플레이어 수 확인 및 게임 종료 체크
	void CheckGameEndCondition();

	// 게임 종료 처리
	void EndGame(bool bIsPlayerWinner);

protected:
	// 독구름(자기장) 시작
	void StartPoisonLogic();

	// 독구름 업데이트 (매 틱 또는 타이머로 호출)
	void UpdatePoisonZone();

	// 독구름 데미지 체크
	void CheckPoisonDamage();

protected:
	// 스폰할 상자 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown")
	TSubclassOf<ABrawlPowerCubeBox> PowerCubeBoxClass;

	// 드랍할 파워 큐브 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown")
	TSubclassOf<ABrawlPowerCube> PowerCubeClass;

	// 독구름 액터 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown|Poison")
	TSubclassOf<class ABrawlPoisonZone> PoisonZoneClass;

	// 독구름 액터 인스턴스
	UPROPERTY()
	TObjectPtr<class ABrawlPoisonZone> PoisonZoneInstance;

	// 독구름 데미지 이펙트 (GE_Poison)
	// SetByCaller로 데미지 량을 전달받을 수 있도록 구성된 GE 클래스를 할당해야 합니다.
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown|Poison")
	TSubclassOf<class UGameplayEffect> PoisonDamageEffectClass;

	// AI로 스폰할 브롤러 클래스 목록 (랜덤 선택)
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown")
	TArray<TSubclassOf<class ABrawlCharacter>> AICharacterClasses;

	// 상자 스폰 개수
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown")
	int32 MaxPowerCubeBoxes = 10;

	// 봇 스폰 개수 (최대 플레이어 수 - 1)
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown")
	int32 MaxBots = 5;

	// 현재 생존한 브롤러 수 (플레이어 + AI)
	UPROPERTY(VisibleInstanceOnly, Category = "Brawl|Showdown")
	int32 AliveBrawlerCount = 0;

	// 쇼다운 모드 전용 AI 행동 트리
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|AI")
	TObjectPtr<class UBehaviorTree> ShowdownAITree;

	// AI 설정 (행동 트리 주입 등)
	void ConfigureAI(class AController* AIController);

	// 독구름 설정
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown|Poison")
	float InitialSafeZoneRadius = 4000.0f; // 초기 안전 구역 반지름 (맵 크기에 맞게 조절)

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown|Poison")
	float MinSafeZoneRadius = 300.0f; // 최소 안전 구역 반지름 (마지막 결전)

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown|Poison")
	float PoisonShrinkSpeed = 50.0f; // 초당 줄어드는 반지름 크기

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown|Poison")
	float PoisonDamage = 1000.0f; // 틱당 데미지

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown|Poison")
	float PoisonDamageInterval = 1.0f; // 데미지 주기 (초)

	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Showdown|Poison")
	float PoisonStartDelay = 5.0f; // 게임 시작 후 독구름 축소 시작 딜레이

	// 내부 변수
	float CurrentSafeZoneRadius;
	FTimerHandle PoisonUpdateTimerHandle;
	FTimerHandle PoisonDamageTimerHandle;
};
