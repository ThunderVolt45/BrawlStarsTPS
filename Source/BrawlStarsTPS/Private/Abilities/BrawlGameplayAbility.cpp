// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility.h"

UBrawlGameplayAbility::UBrawlGameplayAbility()
{
	// 기본적으로 인스턴싱 정책을 'InstancedPerActor'로 설정
	// (필요에 따라 Blueprint에서 변경 가능)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 태그 설정 등 초기화
}

void UBrawlGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 디버그 메시지: 화면과 로그에 출력
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("Ability Activated: %s"), *GetName());
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, DebugMsg);
	}

	UE_LOG(LogTemp, Log, TEXT("BrawlAbility: [%s] Activated on [%s]"), *GetName(), *GetAvatarActorFromActorInfo()->GetName());
}
