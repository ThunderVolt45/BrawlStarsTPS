// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SBrawlLoadingScreen.h"
#include "BrawlGameInstance.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Brushes/SlateImageBrush.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"

void SBrawlLoadingScreen::Construct(const FArguments& InArgs)
{
	// 인자로 전달받은 GameInstance 사용
	UBrawlGameInstance* GI = InArgs._GameInstance;

	// 1. 배경 머티리얼 로드 및 MID 생성
	UMaterialInterface* BaseMat = nullptr;
	if (GI && GI->LoadingBackgroundMaterial)
	{
		BaseMat = GI->LoadingBackgroundMaterial;
	}
	else
	{
		BaseMat = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Game/Materials/M_ScrollingBackground")));
	}
	
	BackgroundMID = nullptr;
	if (BaseMat)
	{
		// GI를 Outer로 사용하여 객체 수명 관리 보강
		BackgroundMID = UMaterialInstanceDynamic::Create(BaseMat, GI);
		if (BackgroundMID)
		{
			// 초기 파라미터 즉시 설정
			BackgroundMID->SetScalarParameterValue(TEXT("UOffset"), CurrentUVOffset.X);
			BackgroundMID->SetScalarParameterValue(TEXT("VOffset"), CurrentUVOffset.Y);
			BackgroundMID->SetScalarParameterValue(TEXT("Alpha"), BackgroundAlpha);
			BackgroundMID->SetScalarParameterValue(TEXT("Tiling"), BackgroundTiling);
			BackgroundMID->SetScalarParameterValue(TEXT("Rotation"), RotationAngle);
		}
	}
	
	// 브러시 생성
	BackgroundBrush = MakeShareable(new FSlateImageBrush(BackgroundMID, FVector2D(1920.0f, 1080.0f)));
	
	// 중앙 로고 및 날개 이미지 텍스처 사용 (프리로드된 것 우선)
	CenterTexture = (GI && GI->LoadingCenterTexture) ? (UObject*)GI->LoadingCenterTexture : 
		StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("/Game/UI/Textures/2239_10x"));
	
	WingTexture = (GI && GI->LoadingWingTexture) ? (UObject*)GI->LoadingWingTexture : 
		StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("/Game/UI/Textures/2238_10x"));
	
	FillerTexture = (GI && GI->LoadingFillerTexture) ? (UObject*)GI->LoadingFillerTexture : 
		StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("/Game/Textures/WhiteDot"));

	CenterBrush = MakeShareable(new FSlateImageBrush(CenterTexture, FVector2D(310.0f, 316.0f)));
	WingBrush = MakeShareable(new FSlateImageBrush(WingTexture, FVector2D(207.5f, 117.0f)));
	FillerBrush = MakeShareable(new FSlateImageBrush(FillerTexture, FVector2D(400.0f, 400.0f)));
	
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
			// 해상도 대응을 위한 스케일 박스
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFill)
			[
				// 기준 해상도 (1920x1080) 설정
				SNew(SBox)
				.WidthOverride(1920.0f)
				.HeightOverride(1080.0f)
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
						.ColorAndOpacity(FLinearColor(1.0f, 0.514918f, 0.014444f, 1.0f))
					]
					
					// 3. 중앙 이미지
					+ SConstraintCanvas::Slot()
					.Anchors(FAnchors(0.5f))
					.Alignment(FVector2D(0.5f, 0.5f))
					.AutoSize(true)
					[
						SAssignNew(CenterImage, SImage)
						.Image(CenterBrush.Get())
					]
				]
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
	if (::IsValid(BackgroundMID))
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
		
		if (Wings[i].IsValid())
		{
			Wings[i]->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
			Wings[i]->SetRenderTransform(FSlateRenderTransform(RotationQuat, Translation));
		}
	}
}

void SBrawlLoadingScreen::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(BackgroundMID);
	Collector.AddReferencedObject(CenterTexture);
	Collector.AddReferencedObject(WingTexture);
	Collector.AddReferencedObject(FillerTexture);
}
