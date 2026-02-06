// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BrawlBrawlerPreview.generated.h"

class USkeletalMeshComponent;
class UDataTable;

/**
 * ABrawlBrawlerPreview
 * 
 * 로비 화면에서 선택된 브롤러를 보여주는 3D 액터입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlBrawlerPreview : public AActor
{
	GENERATED_BODY()
	
public:	
	ABrawlBrawlerPreview();

protected:
	virtual void BeginPlay() override;

	/** 브롤러 변경 시 호출될 콜백 */
	UFUNCTION()
	void OnBrawlerChanged(FName NewBrawlerRowName);

	/** 실제 메시 업데이트 로직 */
	void UpdatePreview(FName BrawlerRowName);

protected:
	/** 현재 생성된 프리뷰 액터 인스턴스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Preview")
	TObjectPtr<AActor> SpawnedPreviewActor;

	/** 캡처를 담당하는 카메라 액터 (에디터에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Preview")
	TObjectPtr<class ASceneCapture2D> CaptureActor;

	/** 브롤러 클래스 정보가 담긴 데이터 테이블 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Data")
	TObjectPtr<UDataTable> BrawlerClassDataTable;

private:
	/** 현재 로드된 메시 핸들 (비동기 로드 대비) */
	FName CurrentBrawlerName;
};
