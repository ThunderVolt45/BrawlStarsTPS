// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BrawlTypes.h"
#include "BrawlGameModeData.generated.h"

/**
 * FBrawlGameModeData
 * 
 * 데이터 테이블(DataTable)에서 게임 모드 정보를 관리하기 위한 구조체입니다.
 */
USTRUCT(BlueprintType)
struct BRAWLSTARSTPS_API FBrawlGameModeData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 게임 모드 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|GameMode")
	FText ModeName;

	// 게임 모드 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|GameMode")
	FText Description;

	// 게임 모드 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|GameMode")
	TSoftObjectPtr<UTexture2D> ModeIcon;

	// 로드할 맵 이름 (예: LV_StormyPlains)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|GameMode")
	FName MapName;

	// 게임 모드 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|GameMode")
	EBrawlGameModeType GameModeType = EBrawlGameModeType::Showdown;

	// 게임 모드 클래스 (필요한 경우)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|GameMode")
	TSoftClassPtr<AGameModeBase> GameModeClass;
};
