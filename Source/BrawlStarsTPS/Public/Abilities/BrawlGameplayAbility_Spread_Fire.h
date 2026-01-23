// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/BrawlGameplayAbility_Fire.h"
#include "BrawlGameplayAbility_Spread_Fire.generated.h"

/**
 * UBrawlGameplayAbility_Spread_Fire
 * 
 * 여러 개의 발사체를 부채꼴 범위로 발사하는 범용 어빌리티 (쉘리 등)
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Spread_Fire : public UBrawlGameplayAbility_Fire
{
	GENERATED_BODY()

protected:
	virtual void SpawnProjectile() override;

protected:
	// 한 번 발사에 생성할 발사체 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Combat")
	int32 ProjectileCount = 5;

	// 탄퍼짐 각도 (도 단위, Horizontal Spread)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Combat")
	float SpreadAngle = 15.0f;
	
	// 각 펠릿(Pellet)당 데미지 비율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Combat")
	float DamagePerPelletScale = 0.2f;
};
