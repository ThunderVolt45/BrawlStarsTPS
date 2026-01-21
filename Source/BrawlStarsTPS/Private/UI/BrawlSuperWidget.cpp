// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlSuperWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UBrawlSuperWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기화
	if (ImageSuperReady)
	{
		ImageSuperReady->SetVisibility(ESlateVisibility::Hidden);
	}
	
	LastPercent = 0.0f;
}

void UBrawlSuperWidget::SetPercent(float InPercent)
{
	// 게이지가 증가했는지 확인 (빛나는 효과 트리거용)
	// 단, 0에서 초기화되는 경우는 제외하거나 기획에 따라 조정
	if (InPercent > LastPercent && InPercent < 1.0f)
	{
		OnChargeIncreased();
	}

	LastPercent = InPercent;
	
	// 부모 클래스의 게이지 업데이트 호출
	Super::SetPercent(InPercent);
}

void UBrawlSuperWidget::SetIsReady(bool bNewIsReady)
{
	Super::SetIsReady(bNewIsReady);

	// 준비 완료 시 '충전 완료' 이미지 표시
	if (ImageSuperReady)
	{
		ImageSuperReady->SetVisibility(bNewIsReady ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	// 준비 완료 상태가 되면 프로그레스 바나 기본 아이콘을 숨길지 여부는 기획에 따름
	// 요구사항: "활성화 상태에서는 비활성화 이미지와 프로그래스바를 가리는 충전 이미지가 나타나야 함"
	// -> Image_SuperReady가 ZOrder 상 위에 있다면 덮어쓰게 되므로 Hidden 처리가 필수는 아닐 수 있음.
	// 하지만 깔끔하게 하기 위해 숨길 수도 있음. 여기서는 가리는(Overlay) 방식이므로 보이게 둠.
}

void UBrawlSuperWidget::OnChargeIncreased()
{
	if (AnimFlash)
	{
		// 애니메이션이 이미 재생 중이면 다시 처음부터 재생
		if (IsAnimationPlaying(AnimFlash))
		{
			StopAnimation(AnimFlash);
		}
		PlayAnimation(AnimFlash);
	}
}
