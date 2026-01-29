// Fill out your copyright notice in the Description page of Project Settings.

#include "BrawlAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "MeshPaintVisualize.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense_Damage.h" // 추가

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
	else if (Attribute == GetReloadSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetSuperChargeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSuperCharge());
	}
	else if (Attribute == GetHyperChargeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHyperCharge());
	}
	else if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetDamageReductionAttribute())
	{
		// 0.0 (0%) ~ 1.0 (100%) 제한
		NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	}
}

void UBrawlAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	UE_LOG(LogTemp, Log, TEXT("PostGE: Attr [%s], Mag [%f], Source [%s], Target [%s]"), 
		*Data.EvaluatedData.Attribute.GetName(), 
		Data.EvaluatedData.Magnitude,
		GetOwningActor() ? *GetOwningActor()->GetName() : TEXT("NULL"), // Target (나)
		Data.EffectSpec.GetContext().GetInstigator() ? *Data.EffectSpec.GetContext().GetInstigator()->GetName() : TEXT("NULL"));

	// 메타 어트리뷰트 IncomingDamage 처리
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.0f); // 처리 후 초기화

		// 0 이하의 데미지는 무시
		if (LocalIncomingDamage <= 0.0f) return;

		// 방어력 적용 (DamageReduction)
		float CurrentReduction = GetDamageReduction(); // 0.0 ~ 1.0
		float ReductionedDamage = LocalIncomingDamage * (1.0f - CurrentReduction);

		// 반올림해서 마무리
		float FinalDamage = FMath::RoundToInt(ReductionedDamage);
		
		// 체력 감소 적용
		float NewHealth = GetHealth() - FinalDamage;
		SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

		// AI Perception에 피격 사실 보고
		AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		AActor* SourceActor = Data.EffectSpec.GetContext().GetInstigator();
		
		if (TargetActor && SourceActor && TargetActor != SourceActor)
		{
			// 공격자가 폰인 경우를 선호하지만, 컨트롤러일 수도 있음. 
			// AI Controller는 Pawn을 통해 팀을 확인하므로 Pawn을 넘기는 것이 좋음.
			AActor* InstigatorActor = SourceActor;
			if (AController* SourceController = Cast<AController>(SourceActor))
			{
				if (APawn* SourcePawn = SourceController->GetPawn())
				{
					InstigatorActor = SourcePawn;
				}
			}

			UAISense_Damage::ReportDamageEvent(
				GetWorld(),
				TargetActor,        // DamagedActor
				InstigatorActor,    // Instigator (Attacker)
				FinalDamage,
				SourceActor->GetActorLocation(), // HitLocation
				FVector::ZeroVector
			);
		}

		// 공격자(Source)에게 게이지 충전
		UAbilitySystemComponent* SourceASC = Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent();
		// AActor* SourceActor = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
		// AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();

		if (SourceActor && TargetActor && SourceActor != TargetActor)
		{
			float ChargeAmount = 0.0f;
			float HyperChargeAmount = 0.0f;

			if (SourceASC)
			{
				bool bFound = false;
				ChargeAmount = SourceASC->GetGameplayAttributeValue(GetSuperChargePerHitAttribute(), bFound);
				HyperChargeAmount = SourceASC->GetGameplayAttributeValue(GetHyperChargePerHitAttribute(), bFound);
			}

			if (ChargeAmount > 0.0f)
			{
				SourceASC->ApplyModToAttributeUnsafe(GetSuperChargeAttribute(), EGameplayModOp::Additive, ChargeAmount);
			}

			// 하이퍼차지 중이 아닐 때만 하이퍼차지 충전
			static FGameplayTag HyperTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
			if (SourceASC && !SourceASC->HasMatchingGameplayTag(HyperTag))
			{
				if (HyperChargeAmount > 0.0f)
				{
					SourceASC->ApplyModToAttributeUnsafe(GetHyperChargeAttribute(), EGameplayModOp::Additive,
					                                     HyperChargeAmount);
				}
			}
		}
		
		return;
	}

	// 체력 처리
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	// 탄약 처리
	else if (Data.EvaluatedData.Attribute == GetAmmoAttribute())
	{
		SetAmmo(FMath::Clamp(GetAmmo(), 0.0f, GetMaxAmmo()));
	}
	// 재장전 시간 처리
	else if (Data.EvaluatedData.Attribute == GetReloadSpeedAttribute())
	{
		SetReloadSpeed(GetReloadSpeed());
	}
	// 궁극기 충전 처리
	else if (Data.EvaluatedData.Attribute == GetSuperChargeAttribute())
	{
		SetSuperCharge(FMath::Clamp(GetSuperCharge(), 0.0f, GetMaxSuperCharge()));
	}
	// 하이퍼 차지 충전 처리
	else if (Data.EvaluatedData.Attribute == GetHyperChargeAttribute())
	{
		SetHyperCharge(FMath::Clamp(GetHyperCharge(), 0.0f, GetMaxHyperCharge()));
	}
	// 이동 속도 처리
	else if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
	{
		SetMovementSpeed(FMath::Max(0.0f, GetMovementSpeed()));
	}
}