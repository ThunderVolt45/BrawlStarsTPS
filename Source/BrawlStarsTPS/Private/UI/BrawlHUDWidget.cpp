// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "UI/BrawlSkillWidget.h"
#include "UI/BrawlGadgetWidget.h"
#include "UI/BrawlSuperWidget.h"
#include "UI/BrawlHyperWidget.h"
#include "GameplayTagContainer.h"

void UBrawlHUDWidget::BindAttributeCallbacks(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;
	
	AbilitySystemComponent = ASC;

	// 1. 체력 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetHealthAttribute()).AddUObject(this, &UBrawlHUDWidget::OnHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UBrawlHUDWidget::OnMaxHealthChanged);

	// 2. 탄환 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetAmmoAttribute()).AddUObject(this, &UBrawlHUDWidget::OnAmmoChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxAmmoAttribute()).AddUObject(this, &UBrawlHUDWidget::OnMaxAmmoChanged);

	// 3. 궁극기 게이지 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetSuperChargeAttribute()).AddUObject(this, &UBrawlHUDWidget::OnSuperChargeChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxSuperChargeAttribute()).AddUObject(this, &UBrawlHUDWidget::OnMaxSuperChargeChanged);

	// 4. 하이퍼차지 게이지 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetHyperChargeAttribute()).AddUObject(this, &UBrawlHUDWidget::OnHyperChargeChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxHyperChargeAttribute()).AddUObject(this, &UBrawlHUDWidget::OnMaxHyperChargeChanged);

	// 초기 값 한 번 보내주기 (위젯 생성 시점)
	const float Health = ASC->GetNumericAttribute(UBrawlAttributeSet::GetHealthAttribute());
	const float MaxHealth = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxHealthAttribute());
	const float Ammo = ASC->GetNumericAttribute(UBrawlAttributeSet::GetAmmoAttribute());
	const float MaxAmmo = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxAmmoAttribute());
	const float SuperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetSuperChargeAttribute());
	const float MaxSuperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxSuperChargeAttribute());
	const float HyperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetHyperChargeAttribute());
	const float MaxHyperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxHyperChargeAttribute());

	OnHealthChangedDelegate.Broadcast(Health);
	OnMaxHealthChangedDelegate.Broadcast(MaxHealth);
	OnAmmoChangedDelegate.Broadcast(Ammo);
	OnMaxAmmoChangedDelegate.Broadcast(MaxAmmo);
	OnSuperChargeChangedDelegate.Broadcast(SuperCharge);
	OnMaxSuperChargeChangedDelegate.Broadcast(MaxSuperCharge);
	OnHyperChargeChangedDelegate.Broadcast(HyperCharge);
	OnMaxHyperChargeChangedDelegate.Broadcast(MaxHyperCharge);

	// 초기 위젯 상태 업데이트
	if (SuperWidget && MaxSuperCharge > 0.f)
	{
		SuperWidget->SetPercent(SuperCharge / MaxSuperCharge);
		SuperWidget->SetIsReady(SuperCharge >= MaxSuperCharge);
	}
	
	if (HyperWidget && MaxHyperCharge > 0.f)
	{
		HyperWidget->SetPercent(HyperCharge / MaxHyperCharge);
		HyperWidget->SetIsReady(HyperCharge >= MaxHyperCharge);
	}
}

void UBrawlHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!AbilitySystemComponent.IsValid()) return;

	// 1. 가젯 쿨다운 처리
	if (Gadget1Widget)
	{
		static FGameplayTag GadgetCooldownTag = FGameplayTag::RequestGameplayTag(FName("State.CooldownGadget"));
		
		FGameplayEffectQuery CooldownQuery;
		CooldownQuery.CustomMatchDelegate.BindLambda([&](const FActiveGameplayEffect& Effect) {
			return Effect.Spec.Def->GetGrantedTags().HasTag(GadgetCooldownTag) || 
				   Effect.Spec.Def->GetAssetTags().HasTag(GadgetCooldownTag);
		});

		TArray<FActiveGameplayEffectHandle> CooldownEffects = AbilitySystemComponent->GetActiveEffects(CooldownQuery);
		if (CooldownEffects.Num() > 0)
		{
			const FActiveGameplayEffect* Effect = AbilitySystemComponent->GetActiveGameplayEffect(CooldownEffects[0]);
			if (Effect)
			{
				float Remaining = Effect->GetTimeRemaining(GetWorld()->GetTimeSeconds());
				float Duration = Effect->GetDuration();
				
				Gadget1Widget->SetRemainingCooldown(Remaining);
				if (Duration > 0.f)
				{
					Gadget1Widget->SetPercent(Remaining / Duration);
				}
			}
		}
		else
		{
			Gadget1Widget->SetRemainingCooldown(0.f);
			Gadget1Widget->SetPercent(0.f);
		}
	}

	// 2. 하이퍼차지 상태 및 지속 시간 처리
	if (HyperWidget)
	{
		static FGameplayTag HyperTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
		bool bIsHyper = AbilitySystemComponent->HasMatchingGameplayTag(HyperTag);
		
		HyperWidget->SetIsActive(bIsHyper);

		if (bIsHyper)
		{
			FGameplayEffectQuery HyperQuery;
			HyperQuery.CustomMatchDelegate.BindLambda([&](const FActiveGameplayEffect& Effect) {
				return Effect.Spec.Def->GetAssetTags().HasTag(HyperTag);
			});
			
			TArray<FActiveGameplayEffectHandle> ActiveEffects = AbilitySystemComponent->GetActiveEffects(HyperQuery);
			if (ActiveEffects.Num() > 0)
			{
				const FActiveGameplayEffect* ActiveGE = AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffects[0]);
				if (ActiveGE)
				{
					float Duration = ActiveGE->GetDuration();
					float Remaining = ActiveGE->GetTimeRemaining(GetWorld()->GetTimeSeconds());
					
					if (Duration > 0.f)
					{
						HyperWidget->SetActivePercent(Remaining / Duration);
					}
				}
			}
		}
	}
}


void UBrawlHUDWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChangedDelegate.Broadcast(Data.NewValue);
}

void UBrawlHUDWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChangedDelegate.Broadcast(Data.NewValue);
}

void UBrawlHUDWidget::OnAmmoChanged(const FOnAttributeChangeData& Data)
{
	OnAmmoChangedDelegate.Broadcast(Data.NewValue);
}

void UBrawlHUDWidget::OnMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
	OnMaxAmmoChangedDelegate.Broadcast(Data.NewValue);
}

void UBrawlHUDWidget::OnSuperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnSuperChargeChangedDelegate.Broadcast(Data.NewValue);

	if (SuperWidget && AbilitySystemComponent.IsValid())
	{
		float MaxVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxSuperChargeAttribute());
		if (MaxVal > 0.f)
		{
			SuperWidget->SetPercent(Data.NewValue / MaxVal);
			SuperWidget->SetIsReady(Data.NewValue >= MaxVal);
		}
	}
}

void UBrawlHUDWidget::OnMaxSuperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnMaxSuperChargeChangedDelegate.Broadcast(Data.NewValue);
	
	// Max값이 바뀌어도 퍼센트는 다시 계산해야 함
	if (SuperWidget && AbilitySystemComponent.IsValid())
	{
		float CurVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetSuperChargeAttribute());
		if (Data.NewValue > 0.f)
		{
			SuperWidget->SetPercent(CurVal / Data.NewValue);
			SuperWidget->SetIsReady(CurVal >= Data.NewValue);
		}
	}
}

void UBrawlHUDWidget::OnHyperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnHyperChargeChangedDelegate.Broadcast(Data.NewValue);

	if (HyperWidget && AbilitySystemComponent.IsValid())
	{
		float MaxVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxHyperChargeAttribute());
		if (MaxVal > 0.f)
		{
			HyperWidget->SetPercent(Data.NewValue / MaxVal);
			HyperWidget->SetIsReady(Data.NewValue >= MaxVal);
		}
	}
}

void UBrawlHUDWidget::OnMaxHyperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHyperChargeChangedDelegate.Broadcast(Data.NewValue);

	if (HyperWidget && AbilitySystemComponent.IsValid())
	{
		float CurVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetHyperChargeAttribute());
		if (Data.NewValue > 0.f)
		{
			HyperWidget->SetPercent(CurVal / Data.NewValue);
			HyperWidget->SetIsReady(CurVal >= Data.NewValue);
		}
	}
}
