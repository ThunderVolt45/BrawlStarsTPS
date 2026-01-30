// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/BrawlGameplayAbility_Fire.h"
#include "BrawlGameplayAbility_Colt_Fire.generated.h"

/**
 * UBrawlGameplayAbility_Colt_Fire
 * 
 * 콜트의 공격 및 발사체 발사를 담당하는 어빌리티
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility_Colt_Fire : public UBrawlGameplayAbility_Fire
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility_Colt_Fire();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// Gameplay Event 수신 시 호출될 콜백
	UFUNCTION()
	void OnFireLeftEventReceived(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnFireRightEventReceived(FGameplayEventData Payload);

	// 발사체 스폰 로직 (AttachParentSocketName: 무기가 붙어있는 부모 소켓 이름, 예: hand_l)
	virtual void SpawnProjectile(FName AttachParentSocketName = NAME_None);

protected:
	// 발사할 발사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<AActor> ProjectileClass;
	
	// 하이퍼차지 상태일 때 사용할 발사체 클래스 (설정되어 있으면 교체됨)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<AActor> ProjectileClass_Hyper;

	// 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> FireMontage;

	// 하이퍼차지 상태일 때 사용할 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> FireMontage_Hyper;

	// 좌측에서 발사체를 생성할 게임플레이 이벤트 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag FireEventTagLeft = FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire_L"));
	
	// 우측에서 발사체를 생성할 게임플레이 이벤트 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag FireEventTagRight = FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire_R"));
	
	// 총이 부착된 소켓
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	FName LeftHandSocket = FName("hand_l");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl")
	FName RightHandSocket = FName("hand_r");
};
