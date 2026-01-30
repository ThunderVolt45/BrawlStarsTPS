// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlKillLogEntry.h"

#include "BrawlCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void UBrawlKillLogEntry::SetKillInfo(AActor* Killer, AActor* Victim)
{
	bool bIsKillerMyself = (Killer == GetOwningPlayerPawn());
	bool bIsVictimMyself = (Victim == GetOwningPlayerPawn());
	
	// 배경 색상을 정하기 위한 기준을 위해 로컬 플레이어를 가져온다
	ABrawlCharacter* LocalPlayer = Cast<ABrawlCharacter>(GetOwningLocalPlayer());
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("LocalPlayer is null! This should not happen! SetKillInfo Halted!"));
		return;
	}
	
	// Killer 브롤러 UI 설정
	if (ABrawlCharacter* KillerBrawler = Cast<ABrawlCharacter>(Killer))
	{
		KillerImage->SetBrushFromTexture(Cast<UTexture2D>(KillerBrawler->GetCharacterIcon()));
		KillerNameText->SetText(FText::FromString(KillerBrawler->GetName()));
		
		// Killer와 로컬 플레이어의 팀을 확인해 알맞은 배경 색상을 설정한다
		if (KillerBrawler->GetTeamID() == LocalPlayer->GetTeamID())
		{
			BackgroundImage->SetBrushTintColor(AllyKillBackgroundColor);
		}
		else
		{
			BackgroundImage->SetBrushTintColor(EnemyKillBackgroundColor);
		}
	}
	
	// Victim 브롤러 UI 설정
	if (ABrawlCharacter* VictimBrawler = Cast<ABrawlCharacter>(Victim))
	{
		VictimImage->SetBrushFromTexture(Cast<UTexture2D>(VictimBrawler->GetCharacterIcon()));
		VictimNameText->SetText(FText::FromString(VictimBrawler->GetName()));
		
		// 만약 Killer가 로컬 플레이어를 처치한 것이라면 배경 색상을 바꿔준다
		if (VictimBrawler == LocalPlayer)
		{
			BackgroundImage->SetBrushTintColor(EnemyKillBackgroundColor);
		}
	}

	// 추가로 처리해야 할 UI 로직이 있다면 블루프린트 상에서 처리한다
	OnKillInfoSet(Killer, Victim, bIsKillerMyself, bIsVictimMyself);
}
