// Fill out your copyright notice in the Description page of Project Settings.

#include "BrawlAttributeSet.h"

#include "BrawlCharacter.h"
#include "GameplayEffectExtension.h"
#include "Perception/AISense_Damage.h" // 추가
#include "BrawlStarsTPSGameMode.h"
#include "Kismet/GameplayStatics.h"

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

	// 메타 어트리뷰트 IncomingDamage 처리
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		OnGetIncomingDamage(Data);
	}
	// 체력 처리
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// 직접 체력이 깎여서 사망하는 경우 미리 처리
		if (GetHealth() <= 0.0f) 
		{
			// 주의: 여기서 GetHealth()는 이미 깎인 값일 수 있음 (Modifiers 적용 후)
			// 하지만 SetHealth로 클램핑하기 전에 체크해야 함
			AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
			AActor* SourceActor = Data.EffectSpec.GetContext().GetInstigator();

			if (ABrawlCharacter* TargetBrawler = Cast<ABrawlCharacter>(TargetActor))
			{
				if (!TargetBrawler->IsDead())
				{
					TargetBrawler->SetLastHitInstigator(SourceActor);
				}
			}
		}

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

void UBrawlAttributeSet::OnGetIncomingDamage(const FGameplayEffectModCallbackData& Data)
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

	// 공격자와 피격자를 가져온다
	AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
	AActor* SourceActor = Data.EffectSpec.GetContext().GetInstigator();

	if (!TargetActor) return;
	if (!SourceActor) return;
	// if (TargetActor == SourceActor) return; // 자해 데미지도 처리 가능하게 주석 해제 고려
	
	// 사망 예정이라면 공격자 정보를 먼저 저장한다 (SetHealth 호출 시 Die가 실행되므로 그 전에 저장 필수)
	if (NewHealth <= 0.0f)
	{
		if (ABrawlCharacter* TargetBrawler = Cast<ABrawlCharacter>(TargetActor))
		{
			TargetBrawler->SetLastHitInstigator(SourceActor);
		}
	}

	// 공격자 정보를 저장한 다음 체력을 갱신한다
	SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
	
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

	// AI Controller에 피격 사실 보고
	UAISense_Damage::ReportDamageEvent(
		GetWorld(),
		TargetActor, // DamagedActor
		InstigatorActor, // Instigator (Attacker)
		FinalDamage,
		SourceActor->GetActorLocation(), // HitLocation
		FVector::ZeroVector
	);
	
	float ChargeAmount = 0.0f;
	float HyperChargeAmount = 0.0f;

	// 공격자의 궁극기, 하이퍼차지 게이지 충전
	UAbilitySystemComponent* SourceASC = Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent();
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
			SourceASC->ApplyModToAttributeUnsafe(GetHyperChargeAttribute(), EGameplayModOp::Additive, HyperChargeAmount);
		}
	}
}
