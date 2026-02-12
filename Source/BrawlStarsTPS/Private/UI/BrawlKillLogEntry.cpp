// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlKillLogEntry.h"

#include "BrawlCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UBrawlKillLogEntry::NativeConstruct()
{
	Super::NativeConstruct();

	// Tick 활성화
	// UUserWidget은 기본적으로 Tick이 꺼져 있을 수 있으므로 켜준다.
	// 하지만 Animation을 위해서라면 보통 TickFrequency를 조정하기도 함.
}

void UBrawlKillLogEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 목표 오프셋 결정
	FVector2D TargetOffset = FVector2D::ZeroVector;

	if (AnimState == EAnimationState::Intro || AnimState == EAnimationState::Idle)
	{
		TargetOffset = FVector2D::ZeroVector; // 원래 위치로
	}
	else if (AnimState == EAnimationState::Outro)
	{
		TargetOffset = SlideOffset; // 다시 밖으로
	}

	// 현재 위치 가져오기
	FVector2D CurrentOffset = GetRenderTransform().Translation;

	// 보간 (Interpolation)
	FVector2D NewOffset = FMath::Vector2DInterpTo(CurrentOffset, TargetOffset, InDeltaTime, AnimationSpeed);

	// 위치 설정
	SetRenderTranslation(NewOffset);

	// Outro 완료 체크
	if (AnimState == EAnimationState::Outro)
	{
		// 목표 지점에 거의 도달했으면 제거
		if (FVector2D::DistSquared(NewOffset, TargetOffset) < 10.0f)
		{
			RemoveFromParent();
		}
	}
}

void UBrawlKillLogEntry::SetKillInfo(AActor* Killer, AActor* Victim)
{
	// 1. 초기 애니메이션 설정
	AnimState = EAnimationState::Intro;
	SetRenderTranslation(SlideOffset); // 시작할 때 밖에서 시작

	// 2. 퇴장 타이머 설정
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(OutroTimerHandle, this, &UBrawlKillLogEntry::StartOutro, DisplayDuration, false);
	}

	if (!KillerImage || !KillerNameText || !VictimImage || !VictimNameText || !BackgroundImage)
	{
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
		
		KillerNameText->SetText(FText::FromString(KillerBrawler->GetName()));
		
		// 로컬 플레이어가 유효할 때만 팀 비교 수행
		if (LocalPlayer)
		{
			if (KillerBrawler->GetTeamID() == LocalPlayer->GetTeamID())
			{
				BackgroundImage->SetBrushTintColor(AllyKillBackgroundColor);
				
				// 아군이 킬을 냈을 때 사운드 재생
				if (GoodKillSFX)
				{
					UGameplayStatics::PlaySound2D(GetWorld(), GoodKillSFX);
				}
			}
			else
			{
				BackgroundImage->SetBrushTintColor(EnemyKillBackgroundColor);
				
				// 적이 아군을 죽였는지 체크 (피해자가 아군 팀일 경우 BadKillSFX)
				if (ABrawlCharacter* VictimBrawler = Cast<ABrawlCharacter>(Victim))
				{
					if (VictimBrawler->GetTeamID() == LocalPlayer->GetTeamID())
					{
						if (BadKillSFX)
						{
							UGameplayStatics::PlaySound2D(GetWorld(), BadKillSFX);
						}
					}
				}
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
		UTexture2D* Icon = VictimBrawler->GetCharacterIcon();
		if (Icon)
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

void UBrawlKillLogEntry::StartOutro()
{
	AnimState = EAnimationState::Outro;
}

