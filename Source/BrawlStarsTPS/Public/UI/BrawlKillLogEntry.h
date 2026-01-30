// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlKillLogEntry.generated.h"

class UImage;
class UTextBlock;
/**
 * 브롤러 처치 로그가 표시될 위젯
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlKillLogEntry : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	// 처치한 브롤러의 아이콘이 표시될 이미지
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> KillerImage;
	
	// 처치한 브롤러의 이름이 표시될 텍스트
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> KillerNameText;
	
	// 처치당한 브롤러의 아이콘이 표시될 이미지
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> VictimImage;
	
	// 처치당한 브롤러의 이름이 표시될 텍스트
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> VictimNameText;
	
	// 위젯의 배경 이미지
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> BackgroundImage;
	
	// 아군 -> 적, 적 -> 적 처치 배경 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	FColor AllyKillBackgroundColor = FColor::Blue;
	
	// 적 -> 아군 처치 배경 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	FColor EnemyKillBackgroundColor = FColor::Red;
	
public:
	// 킬 정보 설정 (액터 전달)
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetKillInfo(AActor* Killer, AActor* Victim);

protected:
	// 블루프린트에서 UI 업데이트 (텍스트 설정, 색상 변경 등)
	// Killer/Victim이 ABrawlCharacter일 수 있으므로 Cast해서 정보 사용
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|UI")
	void OnKillInfoSet(AActor* Killer, AActor* Victim, bool bIsKillerMyself, bool bIsVictimMyself);
};
