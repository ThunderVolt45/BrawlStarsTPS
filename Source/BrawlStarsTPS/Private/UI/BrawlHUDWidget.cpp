// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"

void UBrawlHUDWidget::BindAttributeCallbacks(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

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
	OnHealthChangedDelegate.Broadcast(ASC->GetNumericAttribute(UBrawlAttributeSet::GetHealthAttribute()));
	OnMaxHealthChangedDelegate.Broadcast(ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxHealthAttribute()));
	OnAmmoChangedDelegate.Broadcast(ASC->GetNumericAttribute(UBrawlAttributeSet::GetAmmoAttribute()));
	OnMaxAmmoChangedDelegate.Broadcast(ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxAmmoAttribute()));
	OnSuperChargeChangedDelegate.Broadcast(ASC->GetNumericAttribute(UBrawlAttributeSet::GetSuperChargeAttribute()));
	OnMaxSuperChargeChangedDelegate.Broadcast(ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxSuperChargeAttribute()));
	OnHyperChargeChangedDelegate.Broadcast(ASC->GetNumericAttribute(UBrawlAttributeSet::GetHyperChargeAttribute()));
	OnMaxHyperChargeChangedDelegate.Broadcast(ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxHyperChargeAttribute()));
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
}

void UBrawlHUDWidget::OnMaxSuperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnMaxSuperChargeChangedDelegate.Broadcast(Data.NewValue);
}

void UBrawlHUDWidget::OnHyperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnHyperChargeChangedDelegate.Broadcast(Data.NewValue);
}

void UBrawlHUDWidget::OnMaxHyperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHyperChargeChangedDelegate.Broadcast(Data.NewValue);
}
