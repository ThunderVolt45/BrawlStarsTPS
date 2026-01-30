// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlKillFeedWidget.generated.h"

class UPanelWidget;

/**
 * 킬 로그 전체를 관리하는 컨테이너 위젯
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlKillFeedWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 게임모드로부터 킬 이벤트 수신
	UFUNCTION()
	void HandleBrawlerKilled(AActor* Killer, AActor* Victim);

	// 킬 로그 항목 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	TSubclassOf<UUserWidget> KillLogEntryClass;

	// 로그가 쌓일 컨테이너 (VerticalBox 등)
	// 블루프린트에서 이름이 "KillFeedList"인 패널이 있어야 함
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> KillFeedList;

	// 최대 표시 개수
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|UI")
	int32 MaxLogEntries = 5;
};
