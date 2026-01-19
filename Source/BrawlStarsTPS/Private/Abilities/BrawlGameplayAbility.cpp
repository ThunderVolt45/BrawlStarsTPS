// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UBrawlGameplayAbility::UBrawlGameplayAbility()
{
	// 기본적으로 인스턴싱 정책을 'InstancedPerActor'로 설정
	// (필요에 따라 Blueprint에서 변경 가능)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 태그 설정 등 초기화
}

void UBrawlGameplayAbility::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, FGameplayTag DataTag, float Magnitude)
{
	if (!EffectClass) return;

	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), ContextHandle);
			if (SpecHandle.IsValid())
			{
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTag, Magnitude);
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void UBrawlGameplayAbility::ApplyDamageEffect(AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount)
{
	if (!TargetActor || !DamageEffectClass) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;

	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get())
		{
			FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);
			if (SpecHandle.IsValid())
			{
				// "Data.Damage" 태그를 사용하여 데미지 전달
				static FGameplayTag DataDamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataDamageTag, DamageAmount);
				
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

void UBrawlGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData){
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 디버그 메시지: 화면과 로그에 출력
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("Ability Activated: %s"), *GetName());
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, DebugMsg);
	}

	UE_LOG(LogTemp, Log, TEXT("BrawlAbility: [%s] Activated on [%s]"), *GetName(), *GetAvatarActorFromActorInfo()->GetName());
}
