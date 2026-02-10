// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"

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

	// UE_LOG(LogTemp, Log, TEXT("ApplyEffectToSelf Called by [%s]. Effect: [%s], Tag: [%s], Mag: %f"), 
	// 	*GetName(), *EffectClass->GetName(), *DataTag.ToString(), Magnitude);

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

	// UE_LOG(LogTemp, Log, TEXT("ApplyDamageEffect Called by [%s]. Target: [%s], Effect: [%s], Damage: %f"), 
	// 	*GetName(), *TargetActor->GetName(), *DamageEffectClass->GetName(), DamageAmount);

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

void UBrawlGameplayAbility::PlayGameplayCue(FVector Location, FVector Normal, FGameplayTag CueTag, FRotator RotationOffset)
{
	FGameplayTag TagToUse = CueTag;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
	if (!ASC) return;

	// 매개변수 태그가 없으면 멤버 변수 사용
	if (!TagToUse.IsValid())
	{
		TagToUse = AbilityGameplayCueTag;

		// 하이퍼차지 상태이고, 하이퍼차지용 태그가 설정되어 있다면 교체
		static FGameplayTag HyperStateTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
		if (ASC->HasMatchingGameplayTag(HyperStateTag) && AbilityGameplayCueTag_Hyper.IsValid())
		{
			TagToUse = AbilityGameplayCueTag_Hyper;
		}
	}

	// 매개변수 태그를 가져오지 못했다면 여기서 중단
	if (!TagToUse.IsValid())
		return;

	FGameplayCueParameters Params;

	// 1. 회전 계산 (기본 Normal + BP 설정 오프셋 + 함수 인자 오프셋)
	FRotator FinalRotation = Normal.Rotation();

	if (!GameplayCueRotationOffset.IsZero())
	{
		FinalRotation += GameplayCueRotationOffset;
	}

	if (!RotationOffset.IsZero())
	{
		FinalRotation += RotationOffset;
	}

	Params.Normal = FinalRotation.Vector();

	// 2. 위치 계산 (기본 Location + 회전된 오프셋)
	// 오프셋을 현재 바라보는 방향(FinalRotation) 기준으로 회전시켜 적용
	Params.Location = Location;

	if (!GameplayCueLocationOffset.IsZero())
	{
		Params.Location += FinalRotation.RotateVector(GameplayCueLocationOffset);
	}

	Params.Instigator = GetAvatarActorFromActorInfo();

	ASC->ExecuteGameplayCue(TagToUse, Params);
}

void UBrawlGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// UE_LOG(LogTemp, Warning, TEXT("Ability: [%s] ActivateAbility Called on [%s]"), 
	// 	*GetName(), ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("NULL"));

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

bool UBrawlGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// 사격 어빌리티인 경우 탄환 체크
	static FGameplayTag FireInputTag = FGameplayTag::RequestGameplayTag(FName("InputTag.Ability.Fire"));
	if (StartupInputTag.MatchesTag(FireInputTag))
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			bool bFound = false;
			float CurrentAmmo = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetAmmoAttribute(), bFound);
			
			// UE_LOG(LogTemp, Log, TEXT("Ability [%s] Checking Ammo. Current: %f, Required: %f"), *GetName(), CurrentAmmo, AbilityCostAmount);

			// 탄환이 설정된 코스트 미만이면 사격 불가
			if (bFound && CurrentAmmo < AbilityCostAmount)
			{
				// UE_LOG(LogTemp, Warning, TEXT("Ability [%s] Blocked: Not Enough Ammo!"), *GetName());
				return false;
			}
		}
	}

	return true;
}

void UBrawlGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// UE_LOG(LogTemp, Log, TEXT("ApplyCost Called for Ability [%s]. Amount: %f"), *GetName(), AbilityCostAmount);

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// GE가 있다면 GE로 적용 (SetByCaller)
		if (CostGameplayEffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CostGameplayEffectClass, GetAbilityLevel(), ContextHandle);
			if (SpecHandle.IsValid())
			{
				static FGameplayTag DataCostTag = FGameplayTag::RequestGameplayTag(FName("Data.Cost"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataCostTag, AbilityCostAmount);
				
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
		else
		{
			// GE가 없다면 C++ 직접 차감 (안전하고 빠름)
			// ApplyModToAttributeUnsafe 사용
			ASC->ApplyModToAttributeUnsafe(UBrawlAttributeSet::GetAmmoAttribute(), EGameplayModOp::Additive, -AbilityCostAmount);
		}

		// 화면 출력 (성공 시)
		// if (GEngine)
		// {
		// 	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Cost Applied! -%.0f"), AbilityCostAmount));
		// }
	}
}