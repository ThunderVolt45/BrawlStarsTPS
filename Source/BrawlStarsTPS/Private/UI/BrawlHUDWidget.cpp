// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "UI/BrawlSkillWidget.h"
#include "UI/BrawlGadgetWidget.h"
#include "UI/BrawlSuperWidget.h"
#include "UI/BrawlHyperWidget.h"
#include "GameplayTagContainer.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"

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

	// 초기 값 업데이트
	float Health = ASC->GetNumericAttribute(UBrawlAttributeSet::GetHealthAttribute());
	float MaxHealth = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxHealthAttribute());
	float Ammo = ASC->GetNumericAttribute(UBrawlAttributeSet::GetAmmoAttribute());
	float MaxAmmo = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxAmmoAttribute());
	float SuperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetSuperChargeAttribute());
	float MaxSuperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxSuperChargeAttribute());
	float HyperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetHyperChargeAttribute());
	float MaxHyperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxHyperChargeAttribute());

	// 델리게이트 브로드캐스트 (BP용)
	OnHealthChangedDelegate.Broadcast(Health);
	OnMaxHealthChangedDelegate.Broadcast(MaxHealth);
	OnAmmoChangedDelegate.Broadcast(Ammo);
	OnMaxAmmoChangedDelegate.Broadcast(MaxAmmo);
	OnSuperChargeChangedDelegate.Broadcast(SuperCharge);
	OnMaxSuperChargeChangedDelegate.Broadcast(MaxSuperCharge);
	OnHyperChargeChangedDelegate.Broadcast(HyperCharge);
	OnMaxHyperChargeChangedDelegate.Broadcast(MaxHyperCharge);

	// 위젯 초기화
	if (HealthBar && MaxHealth > 0.f) HealthBar->SetPercent(Health / MaxHealth);
	if (HealthText) HealthText->SetText(FText::AsNumber((int32)Health));
	
	if (AmmoBar && MaxAmmo > 0.f) AmmoBar->SetPercent(Ammo / MaxAmmo);
	if (AmmoText) AmmoText->SetText(FText::Format(
			NSLOCTEXT("BrawlHUD", "AmmoTextFormat", "{0} / {1}"), 
			FText::AsNumber((int32)Ammo), FText::AsNumber((int32)MaxAmmo)));

	// 스킬 위젯 업데이트
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

	// 0. 게임 시간 표시 (GameMode가 있다고 가정)
	// 쇼다운 등의 모드에서는 카운트다운일 수 있음. 여기서는 단순히 서버 시간(초)을 분:초로 표시
	if (MatchTimerText)
	{
		if (UWorld* World = GetWorld())
		{
			// GameState에서 실제 매치 시간을 가져오는 게 좋음.
			// 일단은 게임 실행 후 경과 시간 표시
			float TimeSeconds = World->GetTimeSeconds();
			
			// 카운트다운 방식이라면: MaxTime - TimeSeconds
			// 여기서는 경과 시간 (0:00 -> 0:01 ...)
			int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.f);
			int32 Seconds = FMath::FloorToInt(TimeSeconds) % 60;
			
			MatchTimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, Seconds)));
		}
	}

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
				return Effect.Spec.Def->GetAssetTags().HasTag(HyperTag) || 
					   Effect.Spec.Def->GetGrantedTags().HasTag(HyperTag);
			});
			
			TArray<FActiveGameplayEffectHandle> ActiveEffects = AbilitySystemComponent->GetActiveEffects(HyperQuery);
			if (ActiveEffects.Num() > 0)
			{
				const FActiveGameplayEffect* ActiveGE = AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffects[0]);
				if (ActiveGE)
				{
					float Duration = ActiveGE->GetDuration();
					float Remaining = ActiveGE->GetTimeRemaining(GetWorld()->GetTimeSeconds());
					
					// 종료 임박 시 0으로 보정
					if (Remaining <= 0.1f) Remaining = 0.0f;

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

	if (AbilitySystemComponent.IsValid())
	{
		float MaxVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxHealthAttribute());
		if (MaxVal > 0.f)
		{
			if (HealthBar) HealthBar->SetPercent(Data.NewValue / MaxVal);
		}
		if (HealthText) HealthText->SetText(FText::AsNumber((int32)Data.NewValue));
	}
}

void UBrawlHUDWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChangedDelegate.Broadcast(Data.NewValue);

	if (AbilitySystemComponent.IsValid())
	{
		float CurVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetHealthAttribute());
		if (Data.NewValue > 0.f)
		{
			if (HealthBar) HealthBar->SetPercent(CurVal / Data.NewValue);
		}
	}
}

void UBrawlHUDWidget::OnAmmoChanged(const FOnAttributeChangeData& Data)
{
	OnAmmoChangedDelegate.Broadcast(Data.NewValue);

	if (AbilitySystemComponent.IsValid())
	{
		float MaxVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxAmmoAttribute());
		
		if (MaxVal > 0.f)
		{
			if (AmmoBar) AmmoBar->SetPercent(Data.NewValue / MaxVal);
		}
		
		if (AmmoText) AmmoText->SetText(FText::Format(
			NSLOCTEXT("BrawlHUD", "AmmoTextFormat", "{0} / {1}"), 
			FText::AsNumber((int32)Data.NewValue), FText::AsNumber((int32)MaxVal)));
	}
}

void UBrawlHUDWidget::OnMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
	OnMaxAmmoChangedDelegate.Broadcast(Data.NewValue);

	if (AbilitySystemComponent.IsValid())
	{
		float CurVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetAmmoAttribute());
		
		if (Data.NewValue > 0.f)
		{
			if (AmmoBar) AmmoBar->SetPercent(CurVal / Data.NewValue);
		}
		
		if (AmmoText) AmmoText->SetText(FText::Format(
			NSLOCTEXT("BrawlHUD", "AmmoTextFormat", "{0} / {1}"), 
			FText::AsNumber((int32)CurVal), FText::AsNumber((int32)Data.NewValue)));
	}
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