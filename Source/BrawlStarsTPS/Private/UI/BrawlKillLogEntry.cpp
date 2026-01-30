// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlKillLogEntry.h"

#include "BrawlCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void UBrawlKillLogEntry::SetKillInfo(AActor* Killer, AActor* Victim)
{
	if (!KillerImage) UE_LOG(LogTemp, Error, TEXT("BrawlKillLogEntry: KillerImage is NULL"));
	if (!KillerNameText) UE_LOG(LogTemp, Error, TEXT("BrawlKillLogEntry: KillerNameText is NULL"));
	if (!VictimImage) UE_LOG(LogTemp, Error, TEXT("BrawlKillLogEntry: VictimImage is NULL"));
	if (!VictimNameText) UE_LOG(LogTemp, Error, TEXT("BrawlKillLogEntry: VictimNameText is NULL"));

	if (!KillerImage || !KillerNameText || !VictimImage || !VictimNameText || !BackgroundImage)
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlKillLogEntry: One of BindWidget Failed! BrawlKillLogEntry Halted!"));
		return;
	}

	APawn* OwningPawn = GetOwningPlayerPawn();
	bool bIsKillerMyself = (Killer == OwningPawn);
	bool bIsVictimMyself = (Victim == OwningPawn);
	
	// 배경 색상을 정하기 위한 기준을 위해 로컬 플레이어를 가져온다
	ABrawlCharacter* LocalPlayer = Cast<ABrawlCharacter>(OwningPawn);
	
	// Killer 브롤러 UI 설정
	if (ABrawlCharacter* KillerBrawler = Cast<ABrawlCharacter>(Killer))
	{
		UTexture2D* Icon = KillerBrawler->GetCharacterIcon();
		if (Icon)
		{
			KillerImage->SetBrushFromTexture(Icon);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BrawlKillLogEntry: Killer Icon is NULL for [%s] (ID: %s)"), 
				*KillerBrawler->GetName(), *KillerBrawler->GetCharacterID().ToString());
		}
		
		KillerNameText->SetText(FText::FromString(KillerBrawler->GetName()));
		
		// 로컬 플레이어가 유효할 때만 팀 비교 수행
		if (LocalPlayer)
		{
			if (KillerBrawler->GetTeamID() == LocalPlayer->GetTeamID())
			{
				BackgroundImage->SetBrushTintColor(AllyKillBackgroundColor);
			}
			else
			{
				BackgroundImage->SetBrushTintColor(EnemyKillBackgroundColor);
			}
		}
		else
		{
			// 로컬 플레이어 정보가 없으면 기본적으로 적군(빨강) 색상 사용 (혹은 중립)
			BackgroundImage->SetBrushTintColor(EnemyKillBackgroundColor);
		}
	}
	
	// Victim 브롤러 UI 설정
	if (ABrawlCharacter* VictimBrawler = Cast<ABrawlCharacter>(Victim))
	{
		if (UTexture2D* Icon = VictimBrawler->GetCharacterIcon())
		{
			VictimImage->SetBrushFromTexture(Icon);
		}
		
		VictimNameText->SetText(FText::FromString(VictimBrawler->GetName()));
		
		// 만약 피해자가 로컬 플레이어라면 배경을 적으로 설정 (내가 죽었으므로 강조)
		if (LocalPlayer && VictimBrawler == LocalPlayer)
		{
			BackgroundImage->SetBrushTintColor(EnemyKillBackgroundColor);
		}
	}

	// 추가로 처리해야 할 UI 로직이 있다면 블루프린트 상에서 처리한다
	OnKillInfoSet(Killer, Victim, bIsKillerMyself, bIsVictimMyself);
}
