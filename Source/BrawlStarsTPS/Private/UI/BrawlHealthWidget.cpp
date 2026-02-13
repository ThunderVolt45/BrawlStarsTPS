// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHealthWidget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlCharacter.h"
#include "BrawlPlayerState.h"
#include "BrawlGameState.h"
#include "Data/BrawlTypes.h"
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

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC) return;

	// 1. 팀 색상 설정 (로컬 플레이어 기준)
	bool bTeamIdentified = false;
	bool bIsEnemy = true;

	// 로컬 플레이어 캐릭터 가져오기
	ABrawlCharacter* LocalChar = Cast<ABrawlCharacter>(LocalPC->GetPawn());
	
	if (LocalChar)
	{
		// IsAlly 함수가 Instigator 관계를 포함하므로 소환물 판별 가능
		bool bIsAlly = LocalChar->IsAlly(TargetChar);
		
		if (bIsAlly)
		{
			// 아군인 경우 (본인 또는 본인의 소환물) 즉시 확정
			bIsEnemy = false;
			bTeamIdentified = true;
		}
		else if (LocalChar->GetGenericTeamId() != FGenericTeamId::NoTeam && 
				 TargetChar->GetGenericTeamId() != FGenericTeamId::NoTeam)
		{
			// 둘 다 유효한 팀이 설정된 경우 비교 결과 확정
			bIsEnemy = true; // IsAlly가 false였으므로 적
			bTeamIdentified = true;
		}
		else
		{
			// 둘 중 하나가 255(NoTeam)인 경우: 쇼다운 모드인지 확인
			if (ABrawlGameState* GS = GetWorld()->GetGameState<ABrawlGameState>())
			{
				if (GS->GetGameModeType() == EBrawlGameModeType::Showdown)
				{
					// 쇼다운에서는 255(NoTeam)가 기본이므로, IsAlly가 false면 확실히 적
					bIsEnemy = true;
					bTeamIdentified = true;
				}
			}
		}
	}
	else
	{
		// Pawn이 아직 없는 경우 PlayerState를 통해 팀 확인 시도
		ABrawlPlayerState* LocalPS = LocalPC->GetPlayerState<ABrawlPlayerState>();
		ABrawlPlayerState* TargetPS = TargetChar->GetPlayerState<ABrawlPlayerState>();

		if (LocalPS && TargetPS)
		{
			int32 LocalTeam = LocalPS->GetTeamID();
			int32 TargetTeam = TargetPS->GetTeamID();

			if (LocalTeam != 255 && TargetTeam != 255)
			{
				bIsEnemy = (LocalTeam != TargetTeam);
				bTeamIdentified = true;
			}
		}
	}

	// 팀이 확인되었거나 재시도 횟수를 초과한 경우 색상 적용
	if (bTeamIdentified || PS_RetryCount >= 10)
	{
		OnTeamColorChanged(bIsEnemy);
	}
	else
	{
		// 아직 팀 정보가 불분명하면 잠시 후 재시도
		PS_RetryCount++;
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBrawlHealthWidget::SetupPlayerStateBindings, 0.3f, false);
		return;
	}

	// 2. PlayerState 델리게이트 연결 (Bounty, TieBreaker)
	ABrawlPlayerState* TargetPS = TargetChar->GetPlayerState<ABrawlPlayerState>();
	if (TargetPS)
	{
		// 델리게이트 연결 (Bounty, TieBreaker)
		TargetPS->OnBountyChanged.AddUniqueDynamic(this, &UBrawlHealthWidget::OnBountyChanged);
		UpdateBountyDisplay(TargetPS->GetBounty());

		TargetPS->OnTieBreakerStateChanged.AddUniqueDynamic(this, &UBrawlHealthWidget::OnTieBreakerStateChanged);
		UpdateTieBreakerDisplay(TargetPS->HasTieBreaker());
	}
	else
	{
		// 소환물 등 PlayerState가 없는 경우는 여기서 종료
		// 단, 캐릭터인데 아직 PS가 없는 경우라면 재시도
		if (TargetChar->IsPlayerControlled() && PS_RetryCount < 10)
		{
			PS_RetryCount++;
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBrawlHealthWidget::SetupPlayerStateBindings, 0.5f, false);
		}
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
			// 타이 브레이커가 있으면 숨김, 없으면 현상금 개수가 0보다 클 때만 표시
			if (bHasTieBreaker)
			{
				BountyIcon->SetVisibility(ESlateVisibility::Collapsed);
			}
			else
			{
				int32 CurrentBounty = 0;
				if (ABrawlCharacter* TargetChar = TargetCharacter.Get())
				{
					if (ABrawlPlayerState* PS = TargetChar->GetPlayerState<ABrawlPlayerState>())
					{
						CurrentBounty = PS->GetBounty();
					}
				}
				BountyIcon->SetVisibility(CurrentBounty > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
			}
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
