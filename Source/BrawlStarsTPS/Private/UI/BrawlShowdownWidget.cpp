// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BrawlShowdownWidget.h"
#include "BrawlGameState.h"
#include "Components/TextBlock.h"

void UBrawlShowdownWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 게임 스테이트 가져오기
	if (UWorld* World = GetWorld())
	{
		BrawlGameState = World->GetGameState<ABrawlGameState>();
	}
	
	UpdateAliveCount();
}


void UBrawlShowdownWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// 주기적으로 생존자 수 확인
	UpdateAliveCount();
}


void UBrawlShowdownWidget::UpdateAliveCount()
{
	if (!AliveCountText) return;
	
	int32 CurrentCount = 0;
	
	if (BrawlGameState.IsValid())
	{
		CurrentCount = BrawlGameState->GetAliveBrawlerCount();
	}
	else
	{
		// 게임 스테이트가 아직 없으면 다시 찾기 시도
		if (UWorld* World = GetWorld())
		{
			BrawlGameState = World->GetGameState<ABrawlGameState>();
			
			if (BrawlGameState.IsValid())
			{
				CurrentCount = BrawlGameState->GetAliveBrawlerCount();
			}
		}
	}
	
	// 값이 변했을 때만 텍스트 갱신
	if (CurrentCount != LastAliveCount)
	{
		LastAliveCount = CurrentCount;
		AliveCountText->SetText(FText::AsNumber(CurrentCount));
		
		// 생존자가 적어지면 텍스트 색상 변경 등의 연출 가능
		if (CurrentCount <= 2)
		{
			AliveCountText->SetColorAndOpacity(FLinearColor::Red);
		}
		else
		{
			AliveCountText->SetColorAndOpacity(FLinearColor::White);
		}
	}
}
