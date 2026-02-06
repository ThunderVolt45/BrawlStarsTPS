// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlLobbyWidget.h"
#include "BrawlGameInstance.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "UI/BrawlLobbyPlayerController.h"

void UBrawlLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (ButtonPlay)
	{
		ButtonPlay->OnClicked.AddDynamic(this, &UBrawlLobbyWidget::OnPlayClicked);
	}

	if (ButtonSelectBrawler)
	{
		ButtonSelectBrawler->OnClicked.AddDynamic(this, &UBrawlLobbyWidget::OnBrawlerSelectClicked);
	}

	if (ButtonSelectMode)
	{
		ButtonSelectMode->OnClicked.AddDynamic(this, &UBrawlLobbyWidget::OnModeSelectClicked);
	}

	// 델리게이트 바인딩 및 초기 UI 설정
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->OnBrawlerChanged.AddDynamic(this, &UBrawlLobbyWidget::UpdateSelectedBrawlerUI);
		
		// 초기값으로 UI 업데이트
		UpdateSelectedBrawlerUI(GI->SelectedBrawlerRowName);
	}
}

void UBrawlLobbyWidget::OnBrawlerSelectClicked()
{
	if (ABrawlLobbyPlayerController* PC = Cast<ABrawlLobbyPlayerController>(GetOwningPlayer()))
	{
		PC->ShowBrawlerSelect();
	}
}

void UBrawlLobbyWidget::OnModeSelectClicked()
{
	// 모드 선택창 UI 오픈 로직 (Blueprint에서 주로 처리하거나 C++로 확장 가능)
	// 여기서는 BlueprintImplementableEvent인 UpdateSelectedModeUI 등을 활용하거나
	// 별도의 위젯을 띄우는 로직을 추가할 수 있음
}

void UBrawlLobbyWidget::OnPlayClicked()
{
	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		GI->StartGame();
	}
}

void UBrawlLobbyWidget::UpdateSelectedBrawlerUI(FName NewRowName)
{
	// 브롤러가 변경되었을 때 호출됩니다.
	// 로비 화면의 3D 브롤러 모델을 교체하거나 하는 로직을 이곳에 구현할 수 있습니다.
	UE_LOG(LogTemp, Log, TEXT("UBrawlLobbyWidget::UpdateSelectedBrawlerUI - Selected Brawler changed to: %s"), *NewRowName.ToString());
}