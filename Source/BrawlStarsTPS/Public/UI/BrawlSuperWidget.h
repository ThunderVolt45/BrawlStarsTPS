// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlSkillWidget.h"
#include "BrawlSuperWidget.generated.h"

class UWidgetAnimation;

/**
 * UBrawlSuperWidget
 * 
 * 궁극기(Super) 전용 위젯입니다.
 * 1. 비활성화 시: 아이콘(비활성) + 원형 게이지
 * 2. 충전 시: 게이지 상승 + 빛나는 애니메이션
 * 3. 활성화 시: 활성 아이콘(오버레이) 표시 (게이지 가림)
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlSuperWidget : public UBrawlSkillWidget
{
	GENERATED_BODY()
	
public:
	virtual void SetPercent(float InPercent) override;
	virtual void SetIsReady(bool bNewIsReady) override;

protected:
	virtual void NativeConstruct() override;

	// 게이지가 증가했을 때 호출되는 함수
	void OnChargeIncreased();

protected:
	// 활성화(충전 완료) 시 나타날 이미지 (기존 아이콘/게이지 위를 덮음)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_SuperReady;

	// 게이지가 찰 때 재생할 애니메이션 (깜빡임/빛남)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_Flash;

private:
	float LastPercent = 0.0f;
};
