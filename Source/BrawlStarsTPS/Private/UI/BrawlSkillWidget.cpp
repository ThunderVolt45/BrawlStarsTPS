// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BrawlSkillWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UBrawlSkillWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 프로그레스 바 이미지에 다이내믹 머티리얼이 할당되어 있지 않다면 생성 시도
	if (ImageProgress)
	{
		// 이미 할당된 머티리얼을 기반으로 다이내믹 인스턴스 생성
		if (ImageProgress->GetBrush().GetResourceObject())
		{
			ProgressMaterialDynamic = ImageProgress->GetDynamicMaterial();
		}
	}

	// 초기 상태 업데이트
	SetPercent(0.0f);
	SetIsReady(false);
}

void UBrawlSkillWidget::SetPercent(float InPercent)
{
	// 0~1 범위 클램핑
	float ClampedPercent = FMath::Clamp(InPercent, 0.0f, 1.0f);

	if (ProgressMaterialDynamic)
	{
		ProgressMaterialDynamic->SetScalarParameterValue(MaterialPercentParameterName, ClampedPercent);
	}
}

void UBrawlSkillWidget::SetIsReady(bool bNewIsReady)
{
	bIsReady = bNewIsReady;

	// 준비 상태에 따른 비주얼 처리
	// 예: 아이콘의 색조(Tint) 변경, 오버레이 표시 등

	if (ImageIcon)
	{
		// 쿨다운 중(준비 안됨)이면 약간 어둡게, 준비되면 밝게
		if (bIsReady)
		{
			ImageIcon->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			ImageIcon->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
		}
	}

	if (ImageReadyOverlay)
	{
		ImageReadyOverlay->SetVisibility(bIsReady ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
	
	// 준비 완료 시 프로그레스 바를 꽉 채우거나 숨기는 로직은 디자인에 따라 다름
	// 여기서는 BP에서 디자인적인 처리를 할 수 있도록 상태값만 관리하고,
	// 필요시 BlueprintImplementableEvent를 추가하여 확장이 가능하도록 함.
}

void UBrawlSkillWidget::SetSkillIcon(UTexture2D* InIcon)
{
	if (ImageIcon && InIcon)
	{
		ImageIcon->SetBrushFromTexture(InIcon);
	}
}
