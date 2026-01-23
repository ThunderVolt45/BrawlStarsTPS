// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_AutoHeal.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "TimerManager.h"

UBrawlGameplayAbility_AutoHeal::UBrawlGameplayAbility_AutoHeal()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 감지할 태그 설정 (해당 어빌리티들이 활성화될 때 Owner에게 부여하는 태그여야 함)
	CombatTriggerTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Combat")));
}

void UBrawlGameplayAbility_AutoHeal::ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// 1. 전투 상태 태그 감지 (공격 등)
		for (const FGameplayTag& Tag : CombatTriggerTags)
		{
			// 이미 등록된 태그인지 확인 (중복 방지)
			if (!TagDelegateHandles.Contains(Tag))
			{
				FDelegateHandle DelHandle = ASC->RegisterGameplayTagEvent(
					Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UBrawlGameplayAbility_AutoHeal::OnCombatTagChanged);
				TagDelegateHandles.Add(Tag, DelHandle);
			}
		}

		// 2. 피격 감지 (GE 적용 시)
		OnEffectAppliedDelegateHandle = ASC->OnGameplayEffectAppliedDelegateToSelf.AddUObject(
			this, &UBrawlGameplayAbility_AutoHeal::OnGameplayEffectApplied);

		// 3. 시작 시 비전투 대기 시작
		EnterCombatState();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UBrawlGameplayAbility_AutoHeal::EndAbility(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->OnGameplayEffectAppliedDelegateToSelf.Remove(OnEffectAppliedDelegateHandle);
		
		// 맵에 저장된 모든 핸들 해제
		for (const TPair<FGameplayTag, FDelegateHandle>& Pair : TagDelegateHandles)
		{
			ASC->RegisterGameplayTagEvent(Pair.Key, EGameplayTagEventType::NewOrRemoved).Remove(Pair.Value);
		}
		
		TagDelegateHandles.Empty();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_StartHeal);
		World->GetTimerManager().ClearTimer(TimerHandle_TickHeal);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBrawlGameplayAbility_AutoHeal::OnCombatTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		// 공격 시작 -> 전투 상태 돌입
		EnterCombatState();
	}
}

void UBrawlGameplayAbility_AutoHeal::OnGameplayEffectApplied(UAbilitySystemComponent* Source, 
	const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle ActiveHandle)
{
	// 데미지 태그가 있거나, IncomingDamage 속성을 건드리는지 확인
	static FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
	
	if (Spec.SetByCallerTagMagnitudes.Contains(DamageTag))
	{
		float DamageValue = Spec.GetSetByCallerMagnitude(DamageTag, false, 0.0f);
		if (DamageValue > 0.0f)
		{
			// 데미지를 받음 -> 전투 상태 돌입
			EnterCombatState();
		}
	}
}

void UBrawlGameplayAbility_AutoHeal::EnterCombatState()
{
	UWorld* World = GetWorld();
	check(World);
	
	UE_LOG(LogTemp, Warning, TEXT("AutoHeal: Actor %s EnterCombatState! Auto Heal Halted!"), *GetAvatarActorFromActorInfo()->GetName());

	// 회복 중단
	World->GetTimerManager().ClearTimer(TimerHandle_TickHeal);

	// 비전투 타이머 리셋 (3초 뒤 StartHealing 호출)
	World->GetTimerManager().SetTimer(TimerHandle_StartHeal, this, &UBrawlGameplayAbility_AutoHeal::StartHealing, NoCombatDelay, false);
}

void UBrawlGameplayAbility_AutoHeal::StartHealing()
{
	UWorld* World = GetWorld();
	check(World);
	
	UE_LOG(LogTemp, Warning, TEXT("AutoHeal: Actor %s ExitCombatState! Auto Heal Started!"), *GetAvatarActorFromActorInfo()->GetName());

	// 회복 틱 시작
	World->GetTimerManager().SetTimer(TimerHandle_TickHeal, this, &UBrawlGameplayAbility_AutoHeal::TickHealing, HealInterval, true);
	
	// 첫 틱 즉시 실행
	TickHealing();
}

void UBrawlGameplayAbility_AutoHeal::TickHealing()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("Ability System Component is NULL! This should not happen!"));
		return;
	}

	bool bFoundMax = false;
	float MaxHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxHealthAttribute(), bFoundMax);
	bool bFoundHealth = false;
	float CurrentHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetHealthAttribute(), bFoundHealth);

	if (bFoundMax && bFoundHealth)
	{
		// 체력이 가득 찼으면 무시
		if (CurrentHealth >= MaxHealth) return;

		float HealAmount = MaxHealth * HealPercentPerSec * HealInterval; // 초당 12.5% * 주기

		if (HealEffectClass)
		{
			// GE를 사용하여 회복 (권장)
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);
			
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealEffectClass, GetAbilityLevel(), ContextHandle);
			if (SpecHandle.IsValid())
			{
				// Data.Heal 태그로 회복량 전달
				static FGameplayTag HealTag = FGameplayTag::RequestGameplayTag(FName("Data.Heal"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(HealTag, HealAmount);
				
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
		else
		{
			// GE 클래스가 없으면 Unsafe 방식으로 직접 적용 (백업)
			ASC->ApplyModToAttributeUnsafe(UBrawlAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, HealAmount);
		}
	}
}