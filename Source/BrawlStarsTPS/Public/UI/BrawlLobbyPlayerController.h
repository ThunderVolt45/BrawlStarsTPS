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

protected:
	virtual void BeginPlay() override;

	/** 로비 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	/** 배경 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> BackgroundWidgetClass;

	/** 생성된 로비 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UUserWidget> LobbyWidget;

	/** 생성된 배경 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UUserWidget> BackgroundWidget;
};
