// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BrawlerClassData.generated.h"

/**
 * FBrawlerClassData
 * 
 * 브롤러 ID(RowName)와 실제 생성할 클래스(Blueprint)를 매핑하는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct BRAWLSTARSTPS_API FBrawlerClassData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 실제 게임에서 생성할 브롤러 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler")
	TSubclassOf<class ABrawlCharacter> BrawlerClass;

	// 로비 프리뷰용 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawler|Preview")
	TSubclassOf<AActor> PreviewActorClass;
};
