// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "BrawlAreaEffect.generated.h"

class USphereComponent;
class UDecalComponent;

/**
 * ABrawlAreaEffect
 * 
 * 범위형 지속 효과(장판)의 기본 클래스입니다.
 * - 범위 감지 (BoxComponent)
 * - 바닥 데칼 (DecalComponent)
 * - 지속 시간 관리
 * - 주기적 효과 실행 타이머
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlAreaEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	ABrawlAreaEffect();

protected:
	virtual void BeginPlay() override;

public:
	// 범위 내 액터가 들어왔을 때
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 범위 내 액터가 나갔을 때
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 주기적으로 실행될 함수 (파생 클래스에서 구현)
	virtual void ApplyPeriodicEffect();

protected:
	// 충돌 범위 (기본: 박스)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AreaEffect")
	TObjectPtr<USphereComponent> Area;

	// 바닥 장판 이펙트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AreaEffect")
	TObjectPtr<UDecalComponent> DecalComponent;

	// 장판 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AreaEffect")
	float Duration = 5.0f;

	// 주기적 효과 간격 (초). 0이면 주기적 효과 없음.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AreaEffect")
	float Period = 1.0f;

	// 현재 범위 내에 있는 액터들 목록
	UPROPERTY(VisibleInstanceOnly, Category = "AreaEffect")
	TArray<TObjectPtr<AActor>> OverlappingActors;

	// 주기적 효과 타이머
	FTimerHandle PeriodTimerHandle;

public:
	// 이 장판을 생성한 어빌리티의 스펙 핸들 (데미지 등 정보 포함)
	FGameplayEffectSpecHandle EffectSpecHandle;
};
