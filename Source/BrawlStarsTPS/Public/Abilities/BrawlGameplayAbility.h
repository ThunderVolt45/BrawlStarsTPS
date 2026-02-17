// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BrawlGameplayAbility.generated.h"

/**
 * UBrawlGameplayAbility
 *
 * 이 프로젝트의 모든 어빌리티(평타, 궁극기, 가젯 등)가 상속받는 기본 클래스
 * 쿨다운, 코스트, 입력 태그 처리 등의 공통 로직을 담을 수 있습니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBrawlGameplayAbility();
	
	/** 이 어빌리티가 사용하는 GameplayCue 태그들을 수집합니다 (프리워밍용) */
	virtual void GetGameplayCueTags(FGameplayTagContainer& OutTags) const;

	// 이 어빌리티를 발동시킬 때 사용할 입력 태그 (예: InputTag.Ability.Gadget)
	// UBrawlHeroComponent에서 입력을 처리할 때 식별자로 사용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FGameplayTag StartupInputTag;


protected:
	// 자신에게 GE를 적용하고 SetByCaller 값 설정 (쿨다운, 코스트 등)
	UFUNCTION(BlueprintCallable, Category = "Brawl|Abilities")
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, FGameplayTag DataTag, float Magnitude);

	// 타겟에게 데미지 GE 적용
	UFUNCTION(BlueprintCallable, Category = "Brawl|Abilities")
	void ApplyDamageEffect(AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount);

	/*
	 *  지정된 위치에서 Gameplay Cue를 실행합니다 (SFX/VFX용) 
	 *  CueTag가 유효하지 않으면 기본 설정된 AbilityGameplayCueTag를 사용합니다.
	 *  RotationOffset: 기본 Normal(보통 UpVector 또는 발사 방향)에 추가로 적용할 회전
	 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Abilities")
	void PlayGameplayCue(FVector Location, FVector Normal = FVector::UpVector, FGameplayTag CueTag = FGameplayTag(), FRotator RotationOffset = FRotator::ZeroRotator);
	
protected:
	// 이 어빌리티 발동 시 재생할 Gameplay Cue 태그 (블루프린트에서 설정 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Effects")
	FGameplayTag AbilityGameplayCueTag;

	// 하이퍼차지 상태일 때 재생할 Gameplay Cue 태그 (비워두면 기본 태그 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Effects")
	FGameplayTag AbilityGameplayCueTag_Hyper;

	// Gameplay Cue 재생 위치 오프셋 (로컬 좌표 기준, 소켓 위치에서 추가 이동)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Effects")
	FVector GameplayCueLocationOffset = FVector::ZeroVector;

	// Gameplay Cue 재생 회전 오프셋 (기본 방향에서 추가 회전)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Effects")
	FRotator GameplayCueRotationOffset = FRotator::ZeroRotator;
	
	// 어빌리티 활성화 시 실행될 로직 (C++에서 오버라이드하거나 블루프린트에서 이벤트 그래프 사용)
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 코스트 체크 로직 오버라이드 (탄환 부족 시 사격 차단 등)
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 코스트 적용 로직 오버라이드 (SetByCaller 지원)
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	protected:
	// 이 어빌리티 사용 시 소모할 비용 양 (SetByCaller로 전달됨)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Costs")
	float AbilityCostAmount = 1.0f;
};
