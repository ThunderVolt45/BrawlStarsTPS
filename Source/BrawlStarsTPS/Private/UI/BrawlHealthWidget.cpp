// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHealthWidget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlCharacter.h"
#include "BrawlPlayerState.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBrawlHealthWidget::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	// 대상 캐릭터 저장
	TargetCharacter = Cast<ABrawlCharacter>(ASC->GetAvatarActor());

	// 1. 초기 체력값 설정
	bool bFound = false;
	CurrentHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetHealthAttribute(), bFound);
	CurrentMaxHealth = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxHealthAttribute(), bFound);
	OnHealthChanged(CurrentHealth, CurrentMaxHealth);

	// 2. 어트리뷰트 변경 델리게이트 등록
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetHealthAttribute()).AddUObject(this, &UBrawlHealthWidget::HealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UBrawlHealthWidget::MaxHealthChanged);
	
	// 파워 큐브 개수 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetPowerCubeCountAttribute()).AddUObject(this, &UBrawlHealthWidget::PowerCubeCountChanged);
	
	// 초기 파워 큐브 값 설정
	UpdatePowerCubeDisplay(ASC->GetNumericAttribute(UBrawlAttributeSet::GetPowerCubeCountAttribute()));

	// 3. PlayerState 바인딩 시도 (팀 색상 및 현상금)
	SetupPlayerStateBindings();
}

void UBrawlHealthWidget::SetupPlayerStateBindings()
{
	// 위젯이 붙은 실제 캐릭터 사용
	ABrawlCharacter* TargetChar = TargetCharacter.Get();
	
	if (!TargetChar)
	{
		// 아직 캐릭터가 설정되지 않았다면 잠시 후 재시도
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBrawlHealthWidget::SetupPlayerStateBindings, 0.2f, false);
		return;
	}

	// 1. [백업] PlayerState가 없더라도 일단 폰의 TeamID를 기반으로 색상 먼저 설정
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC)
	{
		// 로컬 플레이어가 조종하는 폰 확인
		if (ABrawlCharacter* LocalChar = Cast<ABrawlCharacter>(LocalPC->GetPawn()))
		{
			uint8 MyTeam = LocalChar->GetTeamID();
			uint8 TargetTeam = TargetChar->GetTeamID();

			bool bIsEnemy = true;
			if (LocalChar == TargetChar) bIsEnemy = false;
			else if (MyTeam != 255 && TargetTeam != 255 && MyTeam == TargetTeam) bIsEnemy = false;

			OnTeamColorChanged(bIsEnemy);
		}
	}

	// 2. [정식] PlayerState가 있다면 델리게이트 연결 및 정밀 판정
	ABrawlPlayerState* TargetPS = TargetChar->GetPlayerState<ABrawlPlayerState>();
	if (TargetPS)
	{
		if (LocalPC)
		{
			ABrawlPlayerState* LocalPS = LocalPC->GetPlayerState<ABrawlPlayerState>();
			if (LocalPS)
			{
				uint8 MyTeamID = LocalPS->GetTeamID();
				uint8 TargetTeamID = TargetPS->GetTeamID();

				bool bIsEnemy = true;
				
				if (LocalPS == TargetPS) bIsEnemy = false;
				else if (MyTeamID != 255 && TargetTeamID != 255 && MyTeamID == TargetTeamID) bIsEnemy = false;

				OnTeamColorChanged(bIsEnemy);
			}
		}

		// 델리게이트 연결 (Bounty, TieBreaker)
		TargetPS->OnBountyChanged.AddUniqueDynamic(this, &UBrawlHealthWidget::OnBountyChanged);
		UpdateBountyDisplay(TargetPS->GetBounty());

		TargetPS->OnTieBreakerStateChanged.AddUniqueDynamic(this, &UBrawlHealthWidget::OnTieBreakerStateChanged);
		UpdateTieBreakerDisplay(TargetPS->HasTieBreaker());
	}
	else
	{
		// PlayerState가 올 때까지 계속 재시도
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBrawlHealthWidget::SetupPlayerStateBindings, 0.2f, false);
	}
}

void UBrawlHealthWidget::OnBountyChanged(int32 NewBounty)
{
	UpdateBountyDisplay(NewBounty);
}

void UBrawlHealthWidget::OnTieBreakerStateChanged(bool bHasTieBreaker)
{
	UpdateTieBreakerDisplay(bHasTieBreaker);
}

void UBrawlHealthWidget::UpdateBountyDisplay(int32 NewBounty)
{
	if (BountyText)
	{
		ESlateVisibility NewVisibility = (NewBounty > 0) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		
		BountyText->SetText(FText::AsNumber(NewBounty));
		BountyText->SetVisibility(NewVisibility);
		
		if (BountyIcon)
		{
			BountyIcon->SetVisibility(NewVisibility);
		}
	}
}

void UBrawlHealthWidget::PowerCubeCountChanged(const FOnAttributeChangeData& Data)
{
	UpdatePowerCubeDisplay(Data.NewValue);
}

void UBrawlHealthWidget::UpdatePowerCubeDisplay(float NewCount)
{
	if (PowerCubeText)
	{
		int32 Count = FMath::RoundToInt(NewCount);
		ESlateVisibility NewVisibility = (Count > 0) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		PowerCubeText->SetText(FText::AsNumber(Count));
		PowerCubeText->SetVisibility(NewVisibility);
		
		if (PowerCubeIcon)
		{
			PowerCubeIcon->SetVisibility(NewVisibility);
		}
	}
}

void UBrawlHealthWidget::UpdateTieBreakerDisplay(bool bHasTieBreaker)
{
	if (TieBreakerIcon)
	{
		TieBreakerIcon->SetVisibility(bHasTieBreaker ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		
		if (BountyIcon)
		{
			BountyIcon->SetVisibility(bHasTieBreaker ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		}
	}
}

void UBrawlHealthWidget::OnHealthChanged(float NewValue, float MaxValue)
{
	if (HealthBar)
	{
		float Percent = (MaxValue > 0.0f) ? (NewValue / MaxValue) : 0.0f;
		HealthBar->SetPercent(Percent);
	}
	
	if (HealthText)
	{
		HealthText->SetText(FText::AsNumber((int32)NewValue));
	}
}

void UBrawlHealthWidget::OnTeamColorChanged(bool bIsEnemy)
{
	if (HealthBar)
	{
		HealthBar->SetFillColorAndOpacity(bIsEnemy ? EnemyHealthBarColor : HealthBarColor);
	}
}

void UBrawlHealthWidget::HealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentHealth = Data.NewValue;
	OnHealthChanged(CurrentHealth, CurrentMaxHealth);
}

void UBrawlHealthWidget::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentMaxHealth = Data.NewValue;
	OnHealthChanged(CurrentHealth, CurrentMaxHealth);
}
