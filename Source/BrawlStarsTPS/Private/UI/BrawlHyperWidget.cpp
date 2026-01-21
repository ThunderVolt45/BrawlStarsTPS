// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHyperWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UBrawlHyperWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Active 이미지의 다이내믹 머티리얼 생성
	if (Image_HyperActive)
	{
		Image_HyperActive->SetVisibility(ESlateVisibility::Hidden);
		if (Image_HyperActive->GetBrush().GetResourceObject())
		{
			ActiveMaterialDynamic = Image_HyperActive->GetDynamicMaterial();
		}
	}

	if (Image_HyperReady)
	{
		Image_HyperReady->SetVisibility(ESlateVisibility::Hidden);
	}

	LastPercent = 0.0f;
	bIsActive = false;
}

void UBrawlHyperWidget::SetPercent(float InPercent)
{
	// 게이지 증가 시 플래시 효과 (Active 상태가 아닐 때만)
	if (!bIsActive && InPercent > LastPercent && InPercent < 1.0f)
	{
		OnChargeIncreased();
	}

	LastPercent = InPercent;
	Super::SetPercent(InPercent);
}

void UBrawlHyperWidget::SetIsReady(bool bNewIsReady)
{
	Super::SetIsReady(bNewIsReady);

	// Active 상태가 아닐 때만 Ready 이미지 표시 제어
	if (!bIsActive)
	{
		if (Image_HyperReady)
		{
			Image_HyperReady->SetVisibility(bNewIsReady ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		}
	}
}

void UBrawlHyperWidget::SetIsActive(bool bNewIsActive)
{
	bIsActive = bNewIsActive;

	if (bIsActive)
	{
		// 발동 상태: Ready와 일반 Progress 숨기고 Active 표시
		if (Image_HyperReady) Image_HyperReady->SetVisibility(ESlateVisibility::Hidden);
		if (Image_Progress) Image_Progress->SetVisibility(ESlateVisibility::Hidden); // 필요하다면 숨김
		if (Image_HyperActive) Image_HyperActive->SetVisibility(ESlateVisibility::HitTestInvisible);
		
		// 아이콘도 숨길지 여부는 디자인에 따라 결정 (여기선 덮어씌우는 컨셉이므로 유지)
	}
	else
	{
		// 발동 해제: 다시 Ready 상태나 일반 상태로 복귀
		if (Image_HyperActive) Image_HyperActive->SetVisibility(ESlateVisibility::Hidden);
		
		// 원래 상태 복원
		if (Image_Progress) Image_Progress->SetVisibility(ESlateVisibility::HitTestInvisible);
		SetIsReady(bIsReady); // bIsReady 값에 따라 Ready 이미지 가시성 재설정
	}
}

void UBrawlHyperWidget::SetActivePercent(float InPercent)
{
	if (ActiveMaterialDynamic)
	{
		ActiveMaterialDynamic->SetScalarParameterValue(MaterialPercentParameterName, InPercent);
	}
}

void UBrawlHyperWidget::OnChargeIncreased()
{
	if (Anim_Flash)
	{
		if (IsAnimationPlaying(Anim_Flash))
		{
			StopAnimation(Anim_Flash);
		}
		PlayAnimation(Anim_Flash);
	}
}
