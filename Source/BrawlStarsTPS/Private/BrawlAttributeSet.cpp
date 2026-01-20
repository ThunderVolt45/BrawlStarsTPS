// Fill out your copyright notice in the Description page of Project Settings.

#include "BrawlAttributeSet.h"
#include "GameplayEffectExtension.h"

UBrawlAttributeSet::UBrawlAttributeSet()
{
}

void UBrawlAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 각 수치들이 변경될 때 자동으로 최소/최대 값 이내로 제한
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAmmo());
	}
	else if (Attribute == GetSuperChargeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSuperCharge());
	}
	else if (Attribute == GetHyperChargeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHyperCharge());
	}
}

void UBrawlAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	UE_LOG(LogTemp, Warning, TEXT("PostGE: Attr [%s], Mag [%f], Source [%s], Target [%s]"), 
		*Data.EvaluatedData.Attribute.GetName(), 
		Data.EvaluatedData.Magnitude,
		*GetOwningActor()->GetName(), // Target (나)
		Data.EffectSpec.GetContext().GetInstigator() ? *Data.EffectSpec.GetContext().GetInstigator()->GetName() : TEXT("NULL"));

	// 각 수치들이 변경될 때 자동으로 최소/최대 값 이내로 제한
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		
		// 데미지를 입혔을 때, 공격자의 궁극기/하이퍼차지 게이지 충전 로직
		if (Data.EvaluatedData.Magnitude < 0.0f) // 데미지(감소)인 경우
		{
			// 공격자(Source)와 피해자(Target) 정보 가져오기
			UAbilitySystemComponent* SourceASC = Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent();
			AActor* SourceActor = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
			AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();

			// 자해(Self-Damage)가 아니고, 공격자가 존재할 때만 충전
			if (!SourceActor || !TargetActor || SourceActor == TargetActor)
				return;
			
			// 실제 입힌 데미지 양 (지금은 충전량 계산에 안 쓰지만 로그용으로 남김)
			const float DamageDealt = FMath::Abs(Data.EvaluatedData.Magnitude);

			// 데미지가 아닌, 명중 당 고정 충전량(PerHit)을 사용
			float ChargeAmount = 0.0f;
			float HyperChargeAmount = 0.0f;

			// 공격자(Source)의 AttributeSet에서 PerHit 값 가져오기
			if (SourceASC)
			{
				bool bFound = false;

				// 만약 못 찾으면 0.0f
				ChargeAmount = SourceASC->GetGameplayAttributeValue(GetSuperChargePerHitAttribute(), bFound);
				HyperChargeAmount = SourceASC->GetGameplayAttributeValue(GetHyperChargePerHitAttribute(), bFound);
			}

			// 공격자의 ASC에 게이지 추가
			if (ChargeAmount > 0.0f)
			{
				SourceASC->ApplyModToAttributeUnsafe(GetSuperChargeAttribute(), EGameplayModOp::Additive, ChargeAmount);
			}
				
			if (HyperChargeAmount > 0.0f)
			{
				SourceASC->ApplyModToAttributeUnsafe(GetHyperChargeAttribute(), EGameplayModOp::Additive, HyperChargeAmount);
			}
			
			// 화면 출력 (디버깅용)
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, 
					FString::Printf(TEXT("Charge! Super +%.0f, Hyper +%.0f"), 
						ChargeAmount, HyperChargeAmount));
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetAmmoAttribute())
	{
		SetAmmo(FMath::Clamp(GetAmmo(), 0.0f, GetMaxAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetSuperChargeAttribute())
	{
		SetSuperCharge(FMath::Clamp(GetSuperCharge(), 0.0f, GetMaxSuperCharge()));
	}
	else if (Data.EvaluatedData.Attribute == GetHyperChargeAttribute())
	{
		SetHyperCharge(FMath::Clamp(GetHyperCharge(), 0.0f, GetMaxHyperCharge()));
	}
}
