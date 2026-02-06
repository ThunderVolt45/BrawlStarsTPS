// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlLobbyBackgroundWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

UBrawlLobbyBackgroundWidget::UBrawlLobbyBackgroundWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBrawlLobbyBackgroundWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ImageBackground)
	{
		// 이미지에 할당된 머티리얼을 기반으로 동적 머티리얼 인스턴스(MID) 생성
		BackgroundMID = ImageBackground->GetDynamicMaterial();
		
		if (!BackgroundMID)
		{
			// 만약 GetDynamicMaterial이 실패하면 명시적으로 생성 시도 (보통 UMG Image는 위 함수로 자동 생성됨)
			UE_LOG(LogTemp, Warning, TEXT("UBrawlLobbyBackgroundWidget: Failed to get Dynamic Material from ImageBackground. Check if a Material is assigned."));
		}
	}
}

void UBrawlLobbyBackgroundWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (BackgroundMID)
	{
		// 오프셋 계산 (시간 * 속도)
		// 1.0을 넘어가면 다시 0으로 초기화하지 않아도 float 정밀도 내에서는 괜찮으나,
		// 장시간 실행 시 정밀도 문제를 방지하기 위해 fmod 등을 쓸 수 있음. 
		// 하지만 텍스처 좌표는 보통 Frac으로 처리되므로 계속 증가시켜도 무방함.
		
		CurrentUVOffset += ScrollSpeed * InDeltaTime;

		// 너무 커지는 것을 방지하기 위해 10000.0f 마다 랩핑 (선택 사항)
		if (CurrentUVOffset.X > 10000.0f) CurrentUVOffset.X -= 10000.0f;
		if (CurrentUVOffset.X < -10000.0f) CurrentUVOffset.X += 10000.0f;
		if (CurrentUVOffset.Y > 10000.0f) CurrentUVOffset.Y -= 10000.0f;
		if (CurrentUVOffset.Y < -10000.0f) CurrentUVOffset.Y += 10000.0f;

		// 머티리얼 파라미터 업데이트
		BackgroundMID->SetScalarParameterValue(TEXT("UOffset"), CurrentUVOffset.X);
		BackgroundMID->SetScalarParameterValue(TEXT("VOffset"), CurrentUVOffset.Y);
		BackgroundMID->SetScalarParameterValue(TEXT("Alpha"), BackgroundAlpha);
		BackgroundMID->SetScalarParameterValue(TEXT("Rotation"), RotationAngle);
		BackgroundMID->SetScalarParameterValue(TEXT("Tiling"), BackgroundTiling);

		// 현재 뷰포트(화면) 비율 계산
		if (APlayerController* PC = GetOwningPlayer())
		{
			int32 ViewportSizeX, ViewportSizeY;
			PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
			
			if (ViewportSizeY > 0)
			{
				float AspectRatio = (float)ViewportSizeX / (float)ViewportSizeY;
				BackgroundMID->SetScalarParameterValue(TEXT("AspectRatio"), AspectRatio);
			}
		}
	}
}
