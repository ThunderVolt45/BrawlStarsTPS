// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BrawlLobbyPlayerController.generated.h"

class UBrawlLobbyWidget;

/**
 * ABrawlLobbyPlayerController
 * 
 * 로비(아웃게임)를 위한 플레이어 컨트롤러입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABrawlLobbyPlayerController();

	/** 로비 화면으로 전환 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void ShowLobby();

	/** 브롤러 선택 화면으로 전환 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void ShowBrawlerSelect();

protected:
	virtual void BeginPlay() override;

	/** 로비 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	/** 배경 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> BackgroundWidgetClass;

	/** 브롤러 선택 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> BrawlerSelectWidgetClass;

	// 로비 배경 음악 오브젝트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Brawl|BGM")
	TObjectPtr<class USoundBase> LobbyBGM;
	
	/** 현재 표시 중인 메인 위젯 (로비 또는 선택창) */
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentMainWidget;

	/** 생성된 배경 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UUserWidget> BackgroundWidget;

private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMComponent;
};
