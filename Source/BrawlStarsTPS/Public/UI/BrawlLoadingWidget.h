// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BrawlLoadingWidget.generated.h"

class UImage;
class UCanvasPanel;

/**
 * UBrawlLoadingWidget
 * 
 * 레벨 이동 시 표시되는 로딩 화면 위젯
 * 중앙 이미지와 주변을 회전하는 6개의 날개로 구성됩니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	/** 중앙 로고 이미지 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CenterImage;

	/** 날개 이미지를 담고 있는 캔버스 패널 (회전용) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> WingsContainer;

	/** 날개 이미지들 (런타임에 자동으로 채워짐) */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Loading")
	TArray<TObjectPtr<UImage>> WingImages;

	/** 회전 속도 */
	UPROPERTY(EditAnywhere, Category = "Loading")
	float RotationSpeed = 100.0f;

	/** 위성 궤도 반지름 */
	UPROPERTY(EditAnywhere, Category = "Loading")
	float OrbitRadius = 150.0f;

private:
	float CurrentRotation = 0.0f;
};
