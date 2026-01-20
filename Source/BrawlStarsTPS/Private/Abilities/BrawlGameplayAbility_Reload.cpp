// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Reload.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "TimerManager.h"

UBrawlGameplayAbility_Reload::UBrawlGameplayAbility_Reload()
{
	// 재장전은 서버/클라이언트 모두 예측 가능하게 동작하거나, 서버 권한으로 동작해야 함
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly; // 안전하게 서버에서만 탄환 관리
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
		TryReloadToken();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UBrawlGameplayAbility_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 델리게이트 해제 및 타이머 정리
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetAmmoAttribute()).RemoveAll(this);
	}
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBrawlGameplayAbility_Reload::OnAmmoAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 탄환이 변할 때마다 재장전 루프를 시작할지 말지 결정
	TryReloadToken();
}

void UBrawlGameplayAbility_Reload::TryReloadToken()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	bool bFoundAmmo = false;
	bool bFoundMaxAmmo = false;
	float CurrentAmmo = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetAmmoAttribute(), bFoundAmmo);
	float MaxAmmo = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxAmmoAttribute(), bFoundMaxAmmo);

	UE_LOG(LogTemp, Warning, TEXT("ReloadAbility Checking: Ammo %f / %f"), CurrentAmmo, MaxAmmo);

	if (bFoundAmmo && bFoundMaxAmmo)
	{
		// 탄환이 부족하면 재장전 타이머 시작
		if (CurrentAmmo < MaxAmmo)
		{
			// 이미 타이머가 돌고 있다면 놔둠
			if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(ReloadTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UBrawlGameplayAbility_Reload::CommitReload, ReloadDuration, false);
			}
		}
		else
		{
			// 꽉 찼으면 타이머 중지
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

	// 실제로 탄환 증가 (GE를 통하지 않고 직접 적용하여 에러 방지)
	// GAS 표준을 위해 GE를 쓰는 것이 좋지만, 지금은 기능 동작이 우선이므로 직접 수정
	// 추후 GE_RestoreAmmo를 제대로 설정하여 ApplyGameplayEffectToSelf를 호출하도록 변경 가능
	
	ASC->ApplyModToAttributeUnsafe(UBrawlAttributeSet::GetAmmoAttribute(), EGameplayModOp::Additive, ReloadAmount);

	// 로그 및 화면 출력
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("Reloaded! +%.0f"), ReloadAmount));
	}
	UE_LOG(LogTemp, Log, TEXT("Reload Complete. Added %f Ammo."), ReloadAmount);

	// 중요: 타이머 콜백 내부에서 다시 타이머를 설정하려면, 
	// 기존 핸들이 아직 'Active'로 간주될 수 있으므로 명시적으로 초기화해야 함.
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	// 재장전 후에도 탄환이 부족하면 다시 타이머 시작
	// 델리게이트에만 의존하지 않고 명시적으로 호출하여 루프 보장
	TryReloadToken();
}
