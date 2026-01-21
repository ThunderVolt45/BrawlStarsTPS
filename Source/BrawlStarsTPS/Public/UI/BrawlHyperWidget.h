// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlSkillWidget.h"
#include "BrawlHyperWidget.generated.h"

class UWidgetAnimation;

/**
 * UBrawlHyperWidget
 * 
 * 하이퍼차지 전용 위젯입니다.
 * 1. 비활성: 아이콘 + 원형 충전 게이지
 * 2. 충전 완료: 충전 완료 아이콘 (Ready)
 * 3. 발동 중: 발동 아이콘 (Active) + 지속 시간 감소 게이지
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlHyperWidget : public UBrawlSkillWidget
{
	GENERATED_BODY()
	
public:
	virtual void SetPercent(float InPercent) override;
	virtual void SetIsReady(bool bNewIsReady) override;

	// 하이퍼차지 발동 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetIsActive(bool bNewIsActive);

	// 발동 중 남은 지속 시간 비율 설정 (1.0 -> 0.0)
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetActivePercent(float InPercent);

protected:
	virtual void NativeConstruct() override;
	
	// 충전량 증가 시 플래시 효과
	void OnChargeIncreased();

protected:
	// 충전 완료 시 표시할 이미지 (Ready)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_HyperReady;

	// 발동 중일 때 표시할 이미지 (Active) - 원형 게이지 머티리얼 사용 권장
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_HyperActive;

	// 충전 시 재생할 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_Flash;

	// 발동 중 게이지 제어를 위한 다이내믹 머티리얼
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ActiveMaterialDynamic;

private:
	float LastPercent = 0.0f;
	bool bIsActive = false;
};
