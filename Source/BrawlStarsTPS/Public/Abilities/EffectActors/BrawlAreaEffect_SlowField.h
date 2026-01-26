// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/EffectActors/BrawlAreaEffect.h"
#include "GameplayEffectTypes.h"
#include "BrawlAreaEffect_SlowField.generated.h"

class UGameplayEffect;

/**
 * ABrawlAreaEffect_SlowField
 * 
 * 스파이크 궁극기 장판:
 * 1. 들어온 적에게 슬로우 효과 (Infinite GE)
 * 2. 범위 내 적에게 주기적 데미지 (Instant GE)
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlAreaEffect_SlowField : public ABrawlAreaEffect
{
	GENERATED_BODY()
	
public:
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	virtual void ApplyPeriodicEffect() override;

protected:
	// 적용할 슬로우 이펙트 클래스 (Infinite 타입 권장)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowField")
	TSubclassOf<UGameplayEffect> SlowEffectClass;

	// 적용할 데미지 이펙트 클래스 (Instant 타입 권장)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SlowField")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 각 액터에게 적용된 슬로우 이펙트 핸들 관리 (나갈 때 제거하기 위해)
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveSlowEffects;
};
