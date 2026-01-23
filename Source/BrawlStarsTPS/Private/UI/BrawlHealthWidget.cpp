// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHealthWidget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBrawlHealthWidget::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlHealthWidget::Initialize - ASC is NULL!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("BrawlHealthWidget::Initialize - ASC Found. Binding Delegates..."));

	// 1. 초기값 설정
	bool bFoundHealth = false;
	CurrentHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetHealthAttribute(), bFoundHealth);
	
	bool bFoundMaxHealth = false;
	CurrentMaxHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxHealthAttribute(), bFoundMaxHealth);

	UE_LOG(LogTemp, Warning, TEXT("BrawlHealthWidget::Initialize - Init Health: %.1f / %.1f"), CurrentHealth, CurrentMaxHealth);

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
	if (HealthBar)
	{
		float Percent = (MaxValue > 0.0f) ? (NewValue / MaxValue) : 0.0f;
		HealthBar->SetPercent(Percent);
	}
	
	if (HealthText)
	{
		HealthText->SetText(FText::AsNumber((int32)NewValue));
	}
}

void UBrawlHealthWidget::OnTeamColorChanged(bool bIsEnemy)
{
	if (HealthBar)
	{
		HealthBar->SetFillColorAndOpacity(bIsEnemy ? EnemyHealthBarColor : HealthBarColor);
	}
}

void UBrawlHealthWidget::HealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Log, TEXT("BrawlHealthWidget::HealthChanged - New: %.1f"), Data.NewValue);
	CurrentHealth = Data.NewValue;
	OnHealthChanged(CurrentHealth, CurrentMaxHealth);
}

void UBrawlHealthWidget::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Log, TEXT("BrawlHealthWidget::MaxHealthChanged - New: %.1f"), Data.NewValue);
	CurrentMaxHealth = Data.NewValue;
	OnHealthChanged(CurrentHealth, CurrentMaxHealth);
}
