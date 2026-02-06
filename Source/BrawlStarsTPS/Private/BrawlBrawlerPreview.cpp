// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlBrawlerPreview.h"
#include "Components/SkeletalMeshComponent.h"
#include "BrawlGameInstance.h"
#include "Data/BrawlerClassData.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"

ABrawlBrawlerPreview::ABrawlBrawlerPreview()
{
	PrimaryActorTick.bCanEverTick = false;
	// MeshComponent 제거됨 (액터 스폰 방식으로 변경)
}

void ABrawlBrawlerPreview::BeginPlay()
{
	Super::BeginPlay();
	
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		// 브롤러 변경 이벤트 구독
		GI->OnBrawlerChanged.AddDynamic(this, &ABrawlBrawlerPreview::OnBrawlerChanged);
		
		// 초기 브롤러로 업데이트
		UpdatePreview(GI->SelectedBrawlerRowName);
	}
}

void ABrawlBrawlerPreview::OnBrawlerChanged(FName NewBrawlerRowName)
{
	UpdatePreview(NewBrawlerRowName);
}

void ABrawlBrawlerPreview::UpdatePreview(FName BrawlerRowName)
{
	if (!BrawlerClassDataTable || BrawlerRowName.IsNone()) return;

	static const FString ContextString(TEXT("Brawler Preview Update"));
	FBrawlerClassData* Row = BrawlerClassDataTable->FindRow<FBrawlerClassData>(BrawlerRowName, ContextString);

	if (Row)
	{
		// 기존 프리뷰 액터 제거
		if (SpawnedPreviewActor)
		{
			SpawnedPreviewActor->Destroy();
			SpawnedPreviewActor = nullptr;
		}

		// 1. 프리뷰 액터 클래스가 지정되어 있다면 스폰
		if (Row->PreviewActorClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			SpawnedPreviewActor = GetWorld()->SpawnActor<AActor>(Row->PreviewActorClass, GetActorTransform(), SpawnParams);
			
			if (SpawnedPreviewActor)
			{
				// 부모 액터에 부착
				SpawnedPreviewActor->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);

				// 카메라의 촬영 목록 업데이트
				if (CaptureActor && CaptureActor->GetCaptureComponent2D())
				{
					USceneCaptureComponent2D* CaptureComp = CaptureActor->GetCaptureComponent2D();
					
					// 기존 목록 비우고 새로운 액터 추가 (또는 기존 꺼 제거 후 추가)
					CaptureComp->ClearShowOnlyComponents(); // 컴포넌트 기반 클리어
					CaptureComp->ShowOnlyActors.Empty();    // 액터 기반 클리어
					CaptureComp->ShowOnlyActors.Add(SpawnedPreviewActor);
				}
			}
		}
	}
}