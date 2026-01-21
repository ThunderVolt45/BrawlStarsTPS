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

	if (HyperWidget && AbilitySystemComponent.IsValid())
	{
		// 하이퍼차지 상태 태그 확인
		static FGameplayTag HyperTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
		
		bool bIsHyper = AbilitySystemComponent->HasMatchingGameplayTag(HyperTag);
		
		// 이전 상태와 비교 로직이 없으므로 매 틱 호출 (최적화 가능)
		// SetIsActive 내부에서 상태 변화 시에만 처리하면 됨
		HyperWidget->SetIsActive(bIsHyper);

		if (bIsHyper)
		{
			// 남은 지속 시간 계산
			// 해당 태그를 부여한 GE를 찾아야 함
			// QueryOptions: TagFilter로 HyperTag를 포함하는 GE 검색
			// 하지만 GetActiveEffectsWithAllTags는 Handle을 반환하지 않고 QueryResult를 반환하므로 조금 복잡함.
			// 간단하게: ASC를 통해 태그에 해당하는 GE의 남은 시간을 가져오는 헬퍼 함수가 있다면 좋음.
			// 여기서는 모든 활성 GE를 순회. (개수가 적으므로 괜찮음)
			
			float Remaining = 0.f;
			float Duration = 0.f;

			// 이 부분은 최적화를 위해 캐싱하거나 다른 방식을 고려할 수 있음
			// 현재는 단순 구현
			FGameplayEffectQuery Query;
			Query.CustomMatchDelegate.BindLambda([&](const FActiveGameplayEffect& Effect) {
				const FGameplayTagContainer& AssetTags = Effect.Spec.Def->GetAssetTags();
				return AssetTags.HasTag(HyperTag);
			});
			
			// 혹은 그냥 GetGameplayEffectRemainingDuration 사용 (만약 태그가 아니라 GE 클래스/핸들을 안다면)
			// 여기서는 태그로 검색
			
			TArray<FActiveGameplayEffectHandle> ActiveEffects = AbilitySystemComponent->GetActiveEffects(Query);
			if (ActiveEffects.Num() > 0)
			{
				// 가장 긴 시간을 가진 것 선택 (중첩 불가라면 하나뿐)
				FActiveGameplayEffectHandle Handle = ActiveEffects[0];
				
				// Remaining과 Duration 얻기
				// ASC의 GetActiveGameplayEffectRemainingDuration 등은 public이 아닐 수 있음.
				// 하지만 FActiveGameplayEffect 객체에서 직접 계산 가능
				
				const FActiveGameplayEffect* ActiveGE = AbilitySystemComponent->GetActiveGameplayEffect(Handle);
				if (ActiveGE)
				{
					Duration = ActiveGE->GetDuration();
					Remaining = ActiveGE->GetTimeRemaining(AbilitySystemComponent->GetWorld()->GetTimeSeconds());
				}
			}

			if (Duration > 0.f)
			{
				HyperWidget->SetActivePercent(Remaining / Duration);
			}
			else
			{
				HyperWidget->SetActivePercent(0.f);
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
