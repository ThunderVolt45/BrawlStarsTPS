// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SBrawlLoadingScreen.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Brushes/SlateImageBrush.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

void SBrawlLoadingScreen::Construct(const FArguments& InArgs)
{
	// 1. 배경 머티리얼 로드 및 MID 생성
	UMaterialInterface* BaseMat = Cast<UMaterialInterface>(
		StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, 
			TEXT("/Game/Materials/M_ScrollingBackground.M_ScrollingBackground")));
	
	if (BaseMat)
	{
		BackgroundMID = UMaterialInstanceDynamic::Create(BaseMat, nullptr);
	}
	
	// 브러시 생성 시 ResourceObject로 MID를 전달합니다.
	BackgroundBrush = MakeShareable(new FSlateImageBrush(BackgroundMID, 
		FVector2D(1920.0f, 1080.0f)));
	
	// 중앙 로고 및 날개 이미지
	CenterBrush = MakeShareable(new FSlateImageBrush(TEXT("/Game/Textures/colt_portrait.colt_portrait"), 
		FVector2D(128.0f, 128.0f)));
	WingBrush = MakeShareable(new FSlateImageBrush(TEXT("/Game/Textures/WhiteDot.WhiteDot"), 
		FVector2D(32.0f, 32.0f)));
	
	// 날개 사이 빈 공간을 채워줄 Filler
	FillerBrush = MakeShareable(new FSlateImageBrush(TEXT("/Game/Textures/WhiteDot.WhiteDot"), 
		FVector2D(350.0f, 350.0f)));
	
	TSharedPtr<SConstraintCanvas> Canvas;
	
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			// 1. 흐르는 배경 머티리얼
			SNew(SImage)
			.Image(BackgroundBrush.Get())
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(Canvas, SConstraintCanvas)
			
			// 2. 날개 뒤 Filler 이미지
			+ SConstraintCanvas::Slot()
			.Anchors(FAnchors(0.5f))
			.Alignment(FVector2D(0.5f, 0.5f))
			.AutoSize(true)
			[
				SAssignNew(FillerImage, SImage)
				.Image(FillerBrush.Get())
				.ColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f)) // 약간 어둡게 처리
			]
			
			// 3. 중앙 이미지
			+ SConstraintCanvas::Slot()
			.Anchors(FAnchors(0.5f))
			.Alignment(FVector2D(0.5f, 0.5f))
			.AutoSize(true)
			[
				SNew(SImage)
				.Image(CenterBrush.Get())
			]
		]
	];
	
	// 4. 6개의 날개 생성 및 배치
	for (int32 i = 0; i < 6; ++i)
	{
		TSharedPtr<SImage> Wing;

		Canvas->AddSlot()
			.Anchors(FAnchors(0.5f))
			.Alignment(FVector2D(0.5f, 0.5f))
			.AutoSize(true)
			[
				SAssignNew(Wing, SImage)
				.Image(WingBrush.Get())
			];
		
		Wings.Add(Wing);
	}
}

void SBrawlLoadingScreen::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// 1. 배경 머티리얼 파라미터 업데이트 (Lobby 위젯 로직 참고)
	if (BackgroundMID)
	{
		CurrentUVOffset += ScrollSpeed * InDeltaTime;
		
		// 랩핑 처리
		if (CurrentUVOffset.X > 10.0f) CurrentUVOffset.X -= 10.0f;
		if (CurrentUVOffset.Y > 10.0f) CurrentUVOffset.Y -= 10.0f;
		
		BackgroundMID->SetScalarParameterValue(TEXT("UOffset"), CurrentUVOffset.X);
		BackgroundMID->SetScalarParameterValue(TEXT("VOffset"), CurrentUVOffset.Y);
		
		// 화면 비율 보정 (필요 시)
		if (AllottedGeometry.GetLocalSize().Y > 0)
		{
			float AspectRatio = AllottedGeometry.GetLocalSize().X / AllottedGeometry.GetLocalSize().Y;
			BackgroundMID->SetScalarParameterValue(TEXT("AspectRatio"), AspectRatio);
		}
	}
	
	CurrentRotation += RotationSpeed * InDeltaTime;
	
	// (필요하다면) Filler도 함께 회전
	// if (FillerImage.IsValid())
	// {
	// 	FillerImage->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	// 	FillerImage->SetRenderTransform(FSlateRenderTransform(
	// 		FQuat2D(FMath::DegreesToRadians(CurrentRotation * 0.5f)), FVector2D::ZeroVector));
	// }
	
	for (int32 i = 0; i < Wings.Num(); ++i)
	{
		float AngleDeg = CurrentRotation + (i * 60.0f);
		float AngleRad = FMath::DegreesToRadians(AngleDeg);
		float X = FMath::Cos(AngleRad) * OrbitRadius;
		float Y = FMath::Sin(AngleRad) * OrbitRadius;
		
		// FQuat2D를 사용하여 회전 생성
		float RotationRad = FMath::DegreesToRadians(AngleDeg + 90.0f);
		FQuat2D RotationQuat(RotationRad);
		FVector2D Translation(X, Y);
		
		Wings[i]->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		Wings[i]->SetRenderTransform(FSlateRenderTransform(RotationQuat, Translation));
	}
}
