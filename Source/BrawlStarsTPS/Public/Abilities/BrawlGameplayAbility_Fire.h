// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/BrawlGameplayAbility.h"
#include "BrawlGameplayAbility_Fire.generated.h"

/**
 * UBrawlGameplayAbility_Fire
 * 
 * 기본 공격 및 발사체 발사를 담당하는 어빌리티
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Fire : public UBrawlGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility_Fire();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// Gameplay Event 수신 시 호출될 콜백
	UFUNCTION()
	void OnFireEventReceived(FGameplayEventData Payload);

	// 발사체 스폰 로직 (Blueprint에서 호출 가능하도록)                                                                      │
	UFUNCTION(BlueprintCallable, Category = "Brawl|Combat")
	void SpawnProjectile();  

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageEnded();

	// 적용할 데미지 어트리뷰트를 반환 (기본값: AttackDamage)
	virtual FGameplayAttribute GetDamageAttribute() const;

protected:
	// 기본 발사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<AActor> ProjectileClass;

	// 하이퍼차지 상태일 때 사용할 발사체 클래스 (설정되어 있으면 교체됨)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<AActor> ProjectileClass_Hyper;

	// 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> FireMontage;

	// 발사체를 생성할 게임플레이 이벤트 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag FireEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire"));
		
	// 발사체 스폰 위치 (소켓 이름)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName MuzzleSocketName = FName("Muzzle");
	
	// 발사체 데미지 (Projectle에 전달할 값)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float DamageAmount = 100.0f;

	// 에임 레이캐스트 최소 사거리 (너무 가까운 곳에 에임이 잡히는 것 방지)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AimMinRange = 800.0f;

	// 에임 레이캐스트 최대 사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AimMaxRange = 10000.0f;

	// 발사체에 적용할 데미지 GE 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;
};
