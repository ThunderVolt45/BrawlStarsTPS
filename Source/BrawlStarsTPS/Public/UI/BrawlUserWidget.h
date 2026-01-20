// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BrawlUserWidget.generated.h"

/**
 * UBrawlUserWidget
 * 
 * 프로젝트의 모든 UI 위젯을 위한 베이스 클래스입니다.
 * 캐릭터나 컨트롤러에 접근하는 공통 함수를 가집니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯이 관리하는 대상 액터(캐릭터 등)를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetWidgetController(UObject* InWidgetController);

protected:
	// 위젯의 주인이 되는 캐릭터나 데이터 객체
	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	TObjectPtr<UObject> WidgetController;

	// 위젯 초기화 시 호출되는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|UI")
	void WidgetControllerSet();
};
