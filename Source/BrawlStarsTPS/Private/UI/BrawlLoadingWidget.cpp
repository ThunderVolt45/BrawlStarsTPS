// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlLoadingWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UBrawlLoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 이름 규칙(Wing_0, Wing_1...)을 사용하여 위젯을 자동으로 찾아 배열에 담음
	WingImages.Empty();
	for (int32 i = 0; i < 6; ++i)
	{
		FName WingName = FName(*FString::Printf(TEXT("Wing_%d"), i));
		if (UImage* FoundWing = Cast<UImage>(GetWidgetFromName(WingName)))
		{
			WingImages.Add(FoundWing);
		}
	}

	if (WingImages.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BrawlLoadingWidget: No Wing widgets found! Make sure to name them Wing_0, Wing_1, etc."));
	}
}

void UBrawlLoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	CurrentRotation += RotationSpeed * InDeltaTime;
	if (CurrentRotation > 360.0f) CurrentRotation -= 360.0f;

	// 6개의 날개를 60도 간격으로 배치하고 회전시킴
	for (int32 i = 0; i < WingImages.Num(); ++i)
	{
		if (UImage* Wing = WingImages[i])
		{
			float AngleDeg = CurrentRotation + (i * (360.0f / 6.0f));
			float AngleRad = FMath::DegreesToRadians(AngleDeg);

			float X = FMath::Cos(AngleRad) * OrbitRadius;
			float Y = FMath::Sin(AngleRad) * OrbitRadius;

			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Wing->Slot))
			{
				CanvasSlot->SetPosition(FVector2D(X, Y));
				
				// 날개 자체가 중앙을 바라보게 하거나, 궤도 방향에 맞춰 회전시키고 싶을 경우
				Wing->SetRenderTransformAngle(AngleDeg + 90.0f); 
			}
		}
	}
}
