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
	UFUNCTION(BlueprintCallable, Category = "Brawl")
	virtual void SpawnProjectile();  

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageEnded();

	// 적용할 데미지 어트리뷰트를 반환 (기본값: AttackDamage)
	virtual FGameplayAttribute GetDamageAttribute() const;

protected:
	// 기본 발사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	TSubclassOf<AActor> ProjectileClass;

	// 하이퍼차지 상태일 때 사용할 발사체 클래스 (설정되어 있으면 교체됨)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	TSubclassOf<AActor> ProjectileClass_Hyper;

	// 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	TObjectPtr<UAnimMontage> FireMontage;

	// 발사체를 생성할 게임플레이 이벤트 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	FGameplayTag FireEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire"));
		
	// 발사체 스폰 위치 (소켓 이름)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	FName MuzzleSocketName = FName("Muzzle");
	
	// 발사체 데미지 (Projectle에 전달할 기본 값)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	float DamageAmount = 100.0f;

	// 에임 레이캐스트 최소 사거리 (너무 가까운 곳에 에임이 잡히는 것 방지)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	float AimMinRange = 700.0f;

	// 에임 레이캐스트 최대 사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	float AimMaxRange = 10000.0f;

	// 발사체에 적용할 데미지 GE 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	// 한 번 발사에 생성할 발사체 개수 (기본값: 1)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	int32 ProjectileCount = 1;

	// 탄퍼짐 각도 (도 단위, Horizontal Spread, 기본값: 0.0f)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	float SpreadAngle = 0.0f;

	// 각 펠릿(Pellet)당 데미지 비율 (기본값: 1.0f, 산탄총의 경우 1보다 작을 수 있음)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	float DamagePerPelletScale = 1.0f;
	
	// AI 컨트롤러가 사용할 경우 에임 보정치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|AI")
	float AIAimOffset = 3.5f;
};
