// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlCharacter.h"
#include "BrawlPowerCubeBox.generated.h"

/**
 * ABrawlPowerCubeBox
 * 
 * 파워 큐브를 드롭하는 상자입니다.
 * AI 타겟팅 및 회전 문제를 해결하기 위해 ABrawlCharacter를 상속받습니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlPowerCubeBox : public ABrawlCharacter
{
	GENERATED_BODY()
	
public:
	ABrawlPowerCubeBox();

	//~ABrawlCharacter interface
	virtual void Die() override;
	//~End of ABrawlCharacter interface

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	// 체력 변경 시 호출
	virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data) override;

protected:
	// AI 감지용 소스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|AI")
	TObjectPtr<class UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;

	// 기본 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	float DefaultMaxHealth = 6000.0f;
	
	// 파괴 시 드롭할 파워 큐브 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Drops")
	TSubclassOf<AActor> PowerCubeClass;

	// 상자 메시 (ABrawlCharacter의 Mesh는 SkeletalMesh이므로 별도의 StaticMesh를 사용하거나 Mesh를 대체)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BoxMeshComponent;

	// 파괴 시 효과
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|FX")
	TSubclassOf<AActor> DestructionEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|FX")
	TObjectPtr<USoundBase> DestructionSFX;

private:
	// ABrawlObstacle에서 사용하던 인터페이스를 직접 구현하거나 필요 기능만 유지
	bool bIsDeadInternal = false;
};
