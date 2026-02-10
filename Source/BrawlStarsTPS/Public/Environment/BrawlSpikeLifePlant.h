// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlCharacter.h"
#include "BrawlSpikeLifePlant.generated.h"

/**
 * ABrawlSpikeLifePlant
 * 
 * 스파이크의 2가젯으로 소환되는 선인장 장애물입니다.
 * 파괴 시 주변 아군을 치유하는 기능을 가집니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlSpikeLifePlant : public ABrawlCharacter
{
	GENERATED_BODY()

public:
	ABrawlSpikeLifePlant();

	//~ABrawlCharacter interface
	virtual void Die() override;
	//~End of ABrawlCharacter interface

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	/** 파괴 시 주변 아군을 치유하는 로직 */
	void HealNearbyAllies();

protected:
	/** 장애물 외형 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlantMeshComponent;

	/** AI 감지용 소스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|AI")
	TObjectPtr<class UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;

	/** 기본 체력 (가젯 소환물 스탯) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	float DefaultMaxHealth = 1750.0f;

	/** 파괴 시 치유량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	float HealAmount = 1000.0f;

	/** 치유에 사용할 Gameplay Effect 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	TSubclassOf<class UGameplayEffect> HealEffectGEClass;

	/** 치유 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	float HealRadius = 400.0f;

	/** 파괴 및 치유 이펙트용 GameplayCue 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|FX")
	FGameplayTag HealCueTag;

private:
	bool bIsDeadInternal = false;
};
