// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlProjectile.h"
#include "BrawlProjectile_Explosive.generated.h"

/**
 * ABrawlProjectile_Explosive
 * 
 * 충돌하거나 수명이 다하면 폭발하며 추가 효과(파편 생성 등)를 발생시키는 발사체
 * (예: 스파이크의 기본 공격)
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlProjectile_Explosive : public ABrawlProjectile
{
	GENERATED_BODY()
	
protected:
	// 충돌 시 호출 (부모의 OnHit을 오버라이드하여 폭발 로직 추가)
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	// 수명 종료(최대 사거리 도달 등) 시 호출
	virtual void Destroyed() override;

	// 폭발 및 파편 생성 로직 실행
	virtual void Explode(const FHitResult& HitResult);

	// 파편 생성
	void SpawnSplinters(const FVector& Location, const FVector& Normal);

protected:
	// 폭발 여부 플래그 (중복 폭발 방지)
	bool bHasExploded = false;

	// --- Config ---

	// 파편으로 생성할 발사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosive")
	TSubclassOf<ABrawlProjectile> SplinterClass;

	// 파편 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosive")
	int32 SplinterCount = 6;

	// 파편 데미지 비율 (본체 데미지 대비)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosive")
	float SplinterDamageScale = 0.5f;

	// 파편이 퍼지는 패턴 (True: 원형/방사형, False: 전방 부채꼴 등... 현재는 방사형 기본)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosive")
	bool bRadialSpread = true;

	// 폭발 시 주변에 줄 데미지 (직격 데미지와 별도, 0이면 적용 안 함)
	// GAS를 통해 적용하려면 복잡해지므로, 일단은 파편 생성에 집중하거나 
	// GameplayEffectClass를 사용하여 Radial Damage를 줄 수 있음.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosive")
	float ExplosionRadius = 0.0f; // 0.0f = 폭발 데미지 없음
};
