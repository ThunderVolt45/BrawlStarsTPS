// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * SBrawlLoadingScreen
 * 
 * MoviePlayer에서 사용할 Slate 기반 로딩 화면 위젯
 */
class SBrawlLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBrawlLoadingScreen) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 애니메이션을 위해 매 프레임 호출
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** 날개들을 회전시키기 위한 변수 */
	float CurrentRotation = 0.0f;
	
	// 날개 궤도 크기
	float OrbitRadius = 218.0f;
	
	// 날개 회전 속도
	float RotationSpeed = 120.0f;

	/** 날개들의 렌더 트랜스폼 데이터 */
	TArray<TSharedPtr<class SImage>> Wings;

	/** 날개 뒤를 채워줄 Filler 이미지 */
	TSharedPtr<class SImage> FillerImage;

	/** 동적 머티리얼 인스턴스 (배경 흐름 제어용) */
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> BackgroundMID;

	/** UV 오프셋 및 설정값 */
	FVector2D CurrentUVOffset = FVector2D::ZeroVector;
	FVector2D ScrollSpeed = FVector2D(0.05f, 0.05f);

	/** 브러시 리소스를 유지하기 위한 포인터들 */
	TSharedPtr<FSlateBrush> BackgroundBrush;
	TSharedPtr<FSlateBrush> CenterBrush;
	TSharedPtr<FSlateBrush> WingBrush;
	TSharedPtr<FSlateBrush> FillerBrush;
};
