// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/BrawlTypes.h"
#include "BrawlGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBrawlerChanged, FName, NewBrawlerRowName);

/**
 * UBrawlGameInstance
 * 
 * 게임 전체의 상태를 관리하는 GameInstance 클래스입니다.
 * 선택된 브롤러, 선택된 게임 모드 등을 유지합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UBrawlGameInstance();

	/** 선택된 브롤러의 RowName (DT_BrawlerData 기준) */
	UPROPERTY(BlueprintReadWrite, Category = "Brawl|State")
	FName SelectedBrawlerRowName = TEXT("Shelly");

	/** 브롤러 변경 시 알림을 위한 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Brawl|Event")
	FOnBrawlerChanged OnBrawlerChanged;

	/** 선택된 게임 모드 정보 */
	UPROPERTY(BlueprintReadWrite, Category = "Brawl|State")
	EBrawlGameModeType SelectedGameModeType = EBrawlGameModeType::Showdown;

	/** 선택된 맵 이름 */
	UPROPERTY(BlueprintReadWrite, Category = "Brawl|State")
	FName SelectedMapName = TEXT("LV_StormyPlains");

	/** 브롤러 설정 및 이벤트 전파 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|State")
	void SetSelectedBrawler(FName NewRowName);

	/** 게임 시작 (선택된 맵으로 이동) */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Game")
	void StartGame();
};
