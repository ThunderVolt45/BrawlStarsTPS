// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Environment/BrawlObstacle.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "BrawlPowerCubeBox.generated.h"

class UAbilitySystemComponent;
class UBrawlAttributeSet;
class UWidgetComponent;

/**
 * ABrawlPowerCubeBox
 * 
 * 파워 큐브를 드롭하는 상자입니다.
 * 일반 장애물(벽)과 달리 체력(Health)을 가지며, GAS를 통해 데미지를 처리합니다.
 * 모든 팀(255)에 대해 중립적 적대 관계를 가집니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlPowerCubeBox : public ABrawlObstacle, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	ABrawlPowerCubeBox();

	//~IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	//~IGenericTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~End of IGenericTeamAgentInterface interface

	//~ABrawlObstacle interface
	virtual bool IsDestructible() const override;
	//~End of ABrawlObstacle interface

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	// 체력 변경 시 호출
	virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data);

	// 사망 처리
	void Die(AActor* Killer);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UBrawlAttributeSet> AttributeSet;

	// 머리 위 체력바 (옵션)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	// AI 감지용 소스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|AI")
	TObjectPtr<class UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;

	// 기본 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	float DefaultMaxHealth = 6000.0f;

	// 팀 ID (기본값 255: 중립)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	uint8 TeamID = 255;
	
	// 파괴 시 드롭할 파워 큐브 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Drops")
	TSubclassOf<AActor> PowerCubeClass;
	
	// 초기화용 Gameplay Effect (속성 초기화용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|GAS")
	TSubclassOf<class UGameplayEffect> InitStatsEffectClass;

private:
	bool bIsDead = false;
};
