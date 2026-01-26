// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/EffectActors/BrawlAreaEffect_SlowField.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void ABrawlAreaEffect_SlowField::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (!OtherActor || !SlowEffectClass) return;

	// 아군은 무시 (Instigator와 같은 팀인지 확인 필요하지만, 일단 Instigator 본인은 슬로우 안 걸리게 예외 처리)
	if (OtherActor == GetInstigator()) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC)
	{
		// 슬로우 GE 적용
		FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		ContextHandle.AddInstigator(GetInstigator(), this);

		FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(SlowEffectClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			
			// 핸들 저장 (나중에 제거용)
			ActiveSlowEffects.Add(OtherActor, ActiveHandle);
		}
	}
}

void ABrawlAreaEffect_SlowField::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex);

	if (!OtherActor) return;

	// 저장된 슬로우 이펙트 제거
	if (FActiveGameplayEffectHandle* HandlePtr = ActiveSlowEffects.Find(OtherActor))
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			TargetASC->RemoveActiveGameplayEffect(*HandlePtr, 1); // 스택 1개 제거
		}
		ActiveSlowEffects.Remove(OtherActor);
	}
}

void ABrawlAreaEffect_SlowField::ApplyPeriodicEffect()
{
	Super::ApplyPeriodicEffect();

	if (!DamageEffectClass) return;

	// 범위 내 모든 액터에게 데미지 적용
	for (AActor* TargetActor : OverlappingActors)
	{
		if (!IsValid(TargetActor)) continue;
		
		// 아군은 무시
		if (TargetActor == GetInstigator()) continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (TargetASC)
		{
			// 데미지 GE 적용
			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);
			ContextHandle.AddInstigator(GetInstigator(), this);

			// 발사체에서 받아온 EffectSpecHandle(데미지 정보)을 사용할 수도 있고,
			// 별도의 DamageEffectClass를 사용할 수도 있음. 
			// 여기서는 DamageEffectClass를 새로 생성해서 적용하는 방식 사용.
			// (만약 부모 어빌리티가 넘겨준 데미지 값을 쓰고 싶다면 EffectSpecHandle을 활용해야 함)
			
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				// 만약 EffectSpecHandle에 저장된 데미지(SetByCaller)가 있다면 복사해올 수 있음
				if (EffectSpecHandle.IsValid())
				{
					static FGameplayTag DataDamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
					float Damage = EffectSpecHandle.Data.Get()->GetSetByCallerMagnitude(DataDamageTag, false, -1.0f);
					if (Damage > 0)
					{
						SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataDamageTag, Damage);
					}
				}

				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
