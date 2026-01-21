// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Reload.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "TimerManager.h"
#include "GameplayTagContainer.h"

UBrawlGameplayAbility_Reload::UBrawlGameplayAbility_Reload()
{
	// 재장전은 서버/클라이언트 모두 예측 가능하게 동작하거나, 서버 권한으로 동작해야 함
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UBrawlGameplayAbility_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// 탄환 변화 감지 델리게이트 등록
		ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetAmmoAttribute()).AddUObject(this, &UBrawlGameplayAbility_Reload::OnAmmoAttributeChanged);
		
		// 시작 시점에 재장전이 필요한지 체크
		FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire"));
		FireTagDelegateHandle = ASC->RegisterGameplayTagEvent(FireTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UBrawlGameplayAbility_Reload::OnFireTagChanged);

		TryReloadToken();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UBrawlGameplayAbility_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 델리게이트 해제
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetAmmoAttribute()).RemoveAll(this);
		
		FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire"));
		ASC->RegisterGameplayTagEvent(FireTag, EGameplayTagEventType::NewOrRemoved).Remove(FireTagDelegateHandle);
	}
	
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBrawlGameplayAbility_Reload::OnAmmoAttributeChanged(const FOnAttributeChangeData& Data)
{
	TryReloadToken();
}

void UBrawlGameplayAbility_Reload::OnFireTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (!GetWorld()) return;
	FTimerManager& TM = GetWorld()->GetTimerManager();

	if (NewCount > 0)
	{
		// 발사 시작 -> 타이머 일시 정지
		if (TM.IsTimerActive(ReloadTimerHandle))
		{
			TM.PauseTimer(ReloadTimerHandle);
		}
	}
	else
	{
		// 발사 종료 -> 타이머 재개
		if (TM.IsTimerPaused(ReloadTimerHandle))
		{
			TM.UnPauseTimer(ReloadTimerHandle);
		}
		else
		{
			// 타이머가 아예 없었다면 새로 시작 체크
			TryReloadToken();
		}
	}
}

void UBrawlGameplayAbility_Reload::TryReloadToken()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;
	
	// 발사 중이면 시작 안 함 (일시정지 상태면 놔둠)
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire"))))
	{
		return;
	}

	bool bFoundAmmo = false;
	bool bFoundMaxAmmo = false;
	float CurrentAmmo = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetAmmoAttribute(), bFoundAmmo);
	float MaxAmmo = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxAmmoAttribute(), bFoundMaxAmmo);

	if (bFoundAmmo && bFoundMaxAmmo)
	{
		if (CurrentAmmo < MaxAmmo)
		{
			if (GetWorld())
			{
				FTimerManager& TM = GetWorld()->GetTimerManager();
				
				// 타이머가 활성화되지 않았고, 일시정지 상태도 아닐 때만 새로 시작
				if (!TM.IsTimerActive(ReloadTimerHandle) && !TM.IsTimerPaused(ReloadTimerHandle))
				{
					TM.SetTimer(ReloadTimerHandle, this, &UBrawlGameplayAbility_Reload::CommitReload, ReloadDuration, false);
				}
			}
		}
		else
		{
			// 꽉 찼으면 타이머 해제
			if (GetWorld())
			{
				GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
			}
		}
	}
}

void UBrawlGameplayAbility_Reload::CommitReload()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	// 발사 중이면 무효 (혹시 모를 안전장치)
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire"))))
	{
		// 이 경우 타이머가 돌았다는 건 Pause가 안 먹혔다는 뜻이므로
		// 다시 Pause 시키거나 다음 기회를 노림
		return;
	}

	ASC->ApplyModToAttributeUnsafe(UBrawlAttributeSet::GetAmmoAttribute(), EGameplayModOp::Additive, ReloadAmount);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	TryReloadToken();
}