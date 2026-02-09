// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlBrawlerButton.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "BrawlGameInstance.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/BrawlLobbyPlayerController.h"

void UBrawlBrawlerButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonSelect)
	{
		ButtonSelect->OnClicked.AddDynamic(this, &UBrawlBrawlerButton::OnButtonClicked);
	}

	RefreshWidget();
}

void UBrawlBrawlerButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshWidget();
}

void UBrawlBrawlerButton::InitializeButton(FName InRowId, FText InDisplayName, TSoftObjectPtr<UTexture2D> InIcon)
{
	BrawlerRowId = InRowId;
	BrawlerName = InDisplayName;
	BrawlerIcon = InIcon;

	RefreshWidget();
}

void UBrawlBrawlerButton::RefreshWidget()
{
	// 배경 색상 설정
	if (ImageBackground)
	{
		ImageBackground->SetColorAndOpacity(BackgroundColor);
	}

	// 텍스트 설정
	if (TextName)
	{
		TextName->SetText(BrawlerName);
	}

	// 아이콘 설정 (좌표 및 크기, 텍스처)
	if (ImageIcon)
	{
		ImageIcon->SetRenderTranslation(IconTranslation);

		if (!BrawlerIcon.IsNull())
		{
			if (UTexture2D* IconTexture = BrawlerIcon.LoadSynchronous())
			{
				if (UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(ImageIcon->Slot))
				{
					ImageSlot->SetSize(IconSize);
				}
				
				ImageIcon->SetBrushFromTexture(IconTexture);
			}
		}
	}
}

void UBrawlBrawlerButton::OnButtonClicked()
{
	// 클릭 사운드 재생 (베이스 클래스 함수 호출)
	PlayClickSFX();

	if (UBrawlGameInstance* GI = Cast<UBrawlGameInstance>(GetGameInstance()))
	{
		// GameInstance에 선택된 브롤러 ID 저장 및 이벤트 전파
		GI->SetSelectedBrawler(BrawlerRowId);
		
		UE_LOG(LogTemp, Log, TEXT("Brawler Selected: %s"), *BrawlerRowId.ToString());

		// 로비 화면으로 복귀
		if (ABrawlLobbyPlayerController* PC = Cast<ABrawlLobbyPlayerController>(GetOwningPlayer()))
		{
			PC->ShowLobby();
		}
	}
}