// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlLobbyBackgroundWidget.generated.h"

class UImage;

/**
 * UBrawlLobbyBackgroundWidget
 * 
 * 로비 배경에서 이미지가 물 흐르듯 이동(Scrolling)하는 효과를 주는 위젯입니다.
 * 배경 이미지에 적용된 머티리얼의 파라미터를 조작하여 구현합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlLobbyBackgroundWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	UBrawlLobbyBackgroundWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	/** 스크롤 속도 (X, Y) - 0.1 정도가 적당함 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Background")
	FVector2D ScrollSpeed = FVector2D(0.05f, 0.05f);

	/** 전체 배경 투명도 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Background")
	float BackgroundAlpha = 1.0f;

	/** 움직이는 레이어의 회전 각도 (도 단위, 0 ~ 360) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Background")
	float RotationAngle = -1.0f;

	/** 움직이는 레이어의 타일링(반복) 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Background")
	float BackgroundTiling = 5.0f;

	/** 머티리얼에서 UV 오프셋을 조절할 파라미터 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Background")
	FName MaterialOffsetParamName = FName("UVOffset");

protected:
	/** 배경 이미지 위젯 (머티리얼이 적용되어 있어야 함) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageBackground;

private:
	/** 동적 머티리얼 인스턴스 */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BackgroundMID;

	/** 현재 누적된 오프셋 값 */
	FVector2D CurrentUVOffset = FVector2D::ZeroVector;
};
