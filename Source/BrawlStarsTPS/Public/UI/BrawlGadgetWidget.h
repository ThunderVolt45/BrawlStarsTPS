// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlSkillWidget.h"
#include "BrawlGadgetWidget.generated.h"

class UTextBlock;

/**
 * UBrawlGadgetWidget
 * 
 * 가젯 전용 위젯입니다.
 * 쿨다운 중일 때 남은 시간을 숫자로 표시하는 기능이 추가되었습니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGadgetWidget : public UBrawlSkillWidget
{
	GENERATED_BODY()
	
public:
	// 남은 쿨다운 시간 설정 및 텍스트 업데이트
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetRemainingCooldown(float RemainingTime);

protected:
	virtual void NativeConstruct() override;

	// 준비 상태 변경 시 텍스트 가시성 처리 등을 위해 오버라이드
	virtual void SetIsReady(bool bNewIsReady) override;

protected:
	// 쿨다운 시간을 표시할 텍스트 블록 (BP에서 이름이 일치해야 함)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextCooldown;
	
	// 가젯 쿨다운 마스크 텍스쳐
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	TObjectPtr<UTexture2D> TextureCooldownMask;
	
	// 가젯 쿨다운 마스크 다이내믹 머티리얼
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ActiveMaterialDynamic;
	
	// 머티리얼의 Percent 파라미터 이름 (기본값: "Percent")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|UI")
	FName MaterialMaskTextureParameterName = FName("MaskTexture");
};
