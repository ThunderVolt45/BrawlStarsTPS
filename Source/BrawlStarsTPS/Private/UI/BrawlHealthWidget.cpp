// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHealthWidget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBrawlHealthWidget::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	// 1. 초기값 설정
	bool bFoundHealth = false;
	CurrentHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetHealthAttribute(), bFoundHealth);
	
	bool bFoundMaxHealth = false;
	CurrentMaxHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxHealthAttribute(), bFoundMaxHealth);

	// 초기 UI 업데이트 호출
	OnHealthChanged(CurrentHealth, CurrentMaxHealth);

	// 2. 어트리뷰트 변경 델리게이트 등록
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetHealthAttribute()).AddUObject(this, &UBrawlHealthWidget::HealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UBrawlHealthWidget::MaxHealthChanged);

	// 3. 팀 색상 결정 (플레이어 컨트롤러 소유 여부 등으로 임시 판단, 추후 팀 시스템 연동)
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		// 이 위젯이 붙은 캐릭터가 로컬 플레이어가 조종하는 캐릭터인가?
		// Overhead Widget은 보통 자기 머리 위에도 뜨고 적 머리 위에도 뜸.
		// WidgetController(캐릭터) 정보를 통해 판단하는 것이 정확함.
		OnTeamColorChanged(false); // 기본은 아군 색상
	}
}

void UBrawlHealthWidget::OnHealthChanged(float NewValue, float MaxValue)
{
	HealthBar->SetPercent(NewValue / MaxValue);
	HealthText->SetText(FText::AsNumber(NewValue));
}

void UBrawlHealthWidget::OnTeamColorChanged(bool bIsEnemy)
{
	HealthBar->SetFillColorAndOpacity(bIsEnemy ? EnemyHealthBarColor : HealthBarColor);
}

void UBrawlHealthWidget::HealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentHealth = Data.NewValue;
	OnHealthChanged(CurrentHealth, CurrentMaxHealth);
}

void UBrawlHealthWidget::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentMaxHealth = Data.NewValue;
	OnHealthChanged(CurrentHealth, CurrentMaxHealth);
}
