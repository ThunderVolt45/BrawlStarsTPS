// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlSkillWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;

/**
 * UBrawlSkillWidget
 * 
 * 스킬(가젯, 슈퍼, 하이퍼차지)의 상태를 표시하는 위젯입니다.
 * 원형 쿨다운/진행바와 아이콘 상태 변화를 제어합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlSkillWidget : public UBrawlUserWidget
{
	GENERATED_BODY()
	
public:
	// 진행도(쿨다운 또는 충전량) 설정 (0.0 ~ 1.0)
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	virtual void SetPercent(float InPercent);

	// 스킬 사용 가능 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	virtual void SetIsReady(bool bIsReady);

	// 스킬 아이콘 텍스처 변경
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	virtual void SetSkillIcon(UTexture2D* InIcon);

protected:
	virtual void NativeConstruct() override;

protected:
	// 원형 게이지를 표시할 이미지 위젯 (BP에서 바인딩 필요)
	// 머티리얼을 사용하여 Circular Progress를 표현합니다.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Progress;

	// 스킬 아이콘을 표시할 이미지 위젯 (BP에서 바인딩 필요)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Icon;

	// 준비 완료 상태일 때 표시할 오버레이 이미지 등 (선택 사항)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_ReadyOverlay;

	// 원형 게이지 제어를 위한 다이내믹 머티리얼 인스턴스
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ProgressMaterialDynamic;

	// 머티리얼의 Percent 파라미터 이름 (기본값: "Percent")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|UI")
	FName MaterialPercentParameterName = FName("Percent");

	// 현재 상태 저장
	bool bIsReady = false;
};
