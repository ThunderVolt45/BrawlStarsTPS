// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BrawlHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "UI/BrawlSkillWidget.h"
#include "UI/BrawlGadgetWidget.h"
#include "UI/BrawlSuperWidget.h"
#include "UI/BrawlHyperWidget.h"
#include "UI/BrawlAmmoSlotWidget.h"
#include "Abilities/BrawlGameplayAbility_Reload.h" // 추가
#include "GameplayTagContainer.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h" // 추가
#include "GameFramework/GameStateBase.h"
#include "Components/PanelWidget.h"
#include "GameFramework/GameModeBase.h"
#include "BrawlCharacter.h"
#include "BrawlPlayerState.h"
#include "Components/Image.h"
#include "Data/BrawlCharacterData.h"

void UBrawlHUDWidget::BindAttributeCallbacks(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;
	
	AbilitySystemComponent = ASC;

	// 1. 체력 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetHealthAttribute()).AddUObject(this, &UBrawlHUDWidget::OnHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UBrawlHUDWidget::OnMaxHealthChanged);

	// 2. 탄환 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetAmmoAttribute()).AddUObject(this, &UBrawlHUDWidget::OnAmmoChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxAmmoAttribute()).AddUObject(this, &UBrawlHUDWidget::OnMaxAmmoChanged);

	// 3. 궁극기 게이지 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetSuperChargeAttribute()).AddUObject(this, &UBrawlHUDWidget::OnSuperChargeChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxSuperChargeAttribute()).AddUObject(this, &UBrawlHUDWidget::OnMaxSuperChargeChanged);

	// 4. 하이퍼차지 게이지 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetHyperChargeAttribute()).AddUObject(this, &UBrawlHUDWidget::OnHyperChargeChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMaxHyperChargeAttribute()).AddUObject(this, &UBrawlHUDWidget::OnMaxHyperChargeChanged);

	// 5. 파워 큐브 변경 감지
	ASC->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetPowerCubeCountAttribute()).AddUObject(this, &UBrawlHUDWidget::OnPowerCubeCountChanged);

	// 초기 값 업데이트
	float Health = ASC->GetNumericAttribute(UBrawlAttributeSet::GetHealthAttribute());
	float MaxHealth = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxHealthAttribute());
	float Ammo = ASC->GetNumericAttribute(UBrawlAttributeSet::GetAmmoAttribute());
	float MaxAmmo = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxAmmoAttribute());
	float SuperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetSuperChargeAttribute());
	float MaxSuperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxSuperChargeAttribute());
	float HyperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetHyperChargeAttribute());
	float MaxHyperCharge = ASC->GetNumericAttribute(UBrawlAttributeSet::GetMaxHyperChargeAttribute());
	float PowerCubes = ASC->GetNumericAttribute(UBrawlAttributeSet::GetPowerCubeCountAttribute());

	// 델리게이트 브로드캐스트 (BP용)
	OnHealthChangedDelegate.Broadcast(Health);
	OnMaxHealthChangedDelegate.Broadcast(MaxHealth);
	OnAmmoChangedDelegate.Broadcast(Ammo);
	OnMaxAmmoChangedDelegate.Broadcast(MaxAmmo);
	OnSuperChargeChangedDelegate.Broadcast(SuperCharge);
	OnMaxSuperChargeChangedDelegate.Broadcast(MaxSuperCharge);
	OnHyperChargeChangedDelegate.Broadcast(HyperCharge);
	OnMaxHyperChargeChangedDelegate.Broadcast(MaxHyperCharge);

	// 위젯 초기화
	if (HealthBar && MaxHealth > 0.f) HealthBar->SetPercent(Health / MaxHealth);
	if (HealthText) HealthText->SetText(FText::AsNumber((int32)Health));
	
	// 초기 탄약 슬롯 업데이트
	UpdateAmmoSlots(Ammo, MaxAmmo);

	// 스킬 위젯 업데이트
	if (SuperWidget && MaxSuperCharge > 0.f)
	{
		SuperWidget->SetPercent(SuperCharge / MaxSuperCharge);
		SuperWidget->SetIsReady(SuperCharge >= MaxSuperCharge);
	}
	
	if (HyperWidget && MaxHyperCharge > 0.f)
	{
		HyperWidget->SetPercent(HyperCharge / MaxHyperCharge);
		HyperWidget->SetIsReady(HyperCharge >= MaxHyperCharge);
	}

	// 파워 큐브 UI 업데이트
	UpdatePowerCubeDisplay(PowerCubes);

	// PlayerState 바인딩 시도 (현상금, 타이 브레이커)
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ABrawlPlayerState* PS = PC->GetPlayerState<ABrawlPlayerState>())
		{
			PS->OnBountyChanged.AddUniqueDynamic(this, &UBrawlHUDWidget::OnBountyChanged);
			PS->OnTieBreakerStateChanged.AddUniqueDynamic(this, &UBrawlHUDWidget::OnTieBreakerStateChanged);
			
			OnBountyChanged(PS->GetBounty());
			OnTieBreakerStateChanged(PS->HasTieBreaker());
		}
	}

	// 게임 모드별 위젯 초기화
	InitializeGameModeWidget();
}

void UBrawlHUDWidget::InitializeBrawlerUI(ABrawlCharacter* Character)
{
	if (!Character) return;

	FBrawlCharacterData Data = Character->GetCharacterData();

	// 가젯 1 아이콘 설정
	if (Gadget1Widget && !Data.Gadget1Icon.IsNull())
	{
		Gadget1Widget->SetSkillIcon(Data.Gadget1Icon.LoadSynchronous());
	}

	// 가젯 2 아이콘 설정 (존재할 경우)
	if (Gadget2Widget && !Data.Gadget2Icon.IsNull())
	{
		Gadget2Widget->SetSkillIcon(Data.Gadget2Icon.LoadSynchronous());
	}

	// 하이퍼차지 아이콘 설정
	if (HyperWidget && !Data.HyperIcon.IsNull())
	{
		HyperWidget->SetSkillIcon(Data.HyperIcon.LoadSynchronous());
	}
	
	// 필요 시 슈퍼(궁극기) 아이콘도 설정 가능하지만, 보통 슈퍼는 고정 아이콘인 경우가 많음
	// 만약 슈퍼 아이콘도 데이터 테이블에서 가져와야 한다면 여기에 추가 가능
}

void UBrawlHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!AbilitySystemComponent.IsValid()) return;

	// 1. 가젯 쿨다운 처리
	if (Gadget1Widget)
	{
		static FGameplayTag GadgetCooldownTag = FGameplayTag::RequestGameplayTag(FName("State.CooldownGadget"));
		
		FGameplayEffectQuery CooldownQuery;
		CooldownQuery.CustomMatchDelegate.BindLambda([&](const FActiveGameplayEffect& Effect) {
			return Effect.Spec.Def->GetGrantedTags().HasTag(GadgetCooldownTag) || 
				   Effect.Spec.Def->GetAssetTags().HasTag(GadgetCooldownTag);
		});

		TArray<FActiveGameplayEffectHandle> CooldownEffects = AbilitySystemComponent->GetActiveEffects(CooldownQuery);
		if (CooldownEffects.Num() > 0)
		{
			const FActiveGameplayEffect* Effect = AbilitySystemComponent->GetActiveGameplayEffect(CooldownEffects[0]);
			if (Effect)
			{
				float Remaining = Effect->GetTimeRemaining(GetWorld()->GetTimeSeconds());
				float Duration = Effect->GetDuration();
				
				Gadget1Widget->SetRemainingCooldown(Remaining);
				if (Duration > 0.f)
				{
					Gadget1Widget->SetPercent(Remaining / Duration);
				}
			}
		}
		else
		{
			Gadget1Widget->SetRemainingCooldown(0.f);
			Gadget1Widget->SetPercent(0.f);
		}
	}

	// 1-2. 가젯 2 쿨다운 처리
	if (Gadget2Widget)
	{
		static FGameplayTag Gadget2CooldownTag = FGameplayTag::RequestGameplayTag(FName("State.CooldownGadget2"));
		
		FGameplayEffectQuery CooldownQuery;
		CooldownQuery.CustomMatchDelegate.BindLambda([&](const FActiveGameplayEffect& Effect) {
			return Effect.Spec.Def->GetGrantedTags().HasTag(Gadget2CooldownTag) || 
				   Effect.Spec.Def->GetAssetTags().HasTag(Gadget2CooldownTag);
		});

		TArray<FActiveGameplayEffectHandle> CooldownEffects = AbilitySystemComponent->GetActiveEffects(CooldownQuery);
		if (CooldownEffects.Num() > 0)
		{
			const FActiveGameplayEffect* Effect = AbilitySystemComponent->GetActiveGameplayEffect(CooldownEffects[0]);
			if (Effect)
			{
				float Remaining = Effect->GetTimeRemaining(GetWorld()->GetTimeSeconds());
				float Duration = Effect->GetDuration();
				
				Gadget2Widget->SetRemainingCooldown(Remaining);
				if (Duration > 0.f)
				{
					Gadget2Widget->SetPercent(Remaining / Duration);
				}
			}
		}
		else
		{
			Gadget2Widget->SetRemainingCooldown(0.f);
			Gadget2Widget->SetPercent(0.f);
		}
	}

	// 2. 하이퍼차지 상태 및 지속 시간 처리
	if (HyperWidget)
	{
		static FGameplayTag HyperTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
		bool bIsHyper = AbilitySystemComponent->HasMatchingGameplayTag(HyperTag);
		
		HyperWidget->SetIsActive(bIsHyper);

		if (bIsHyper)
		{
			FGameplayEffectQuery HyperQuery;
			HyperQuery.CustomMatchDelegate.BindLambda([&](const FActiveGameplayEffect& Effect) {
				return Effect.Spec.Def->GetAssetTags().HasTag(HyperTag) || 
					   Effect.Spec.Def->GetGrantedTags().HasTag(HyperTag);
			});
			
			TArray<FActiveGameplayEffectHandle> ActiveEffects = AbilitySystemComponent->GetActiveEffects(HyperQuery);
			if (ActiveEffects.Num() > 0)
			{
				const FActiveGameplayEffect* ActiveGE = AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffects[0]);
				if (ActiveGE)
				{
					float Duration = ActiveGE->GetDuration();
					float Remaining = ActiveGE->GetTimeRemaining(GetWorld()->GetTimeSeconds());
					
					// 종료 임박 시 0으로 보정
					if (Remaining <= 0.1f) Remaining = 0.0f;

					if (Duration > 0.f)
					{
						HyperWidget->SetActivePercent(Remaining / Duration);
					}
				}
			}
		}
	}

	// 3. 탄약 슬롯 부드러운 업데이트
	float Ammo = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetAmmoAttribute());
	float MaxAmmo = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxAmmoAttribute());
	UpdateAmmoSlots(Ammo, MaxAmmo);
}

void UBrawlHUDWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChangedDelegate.Broadcast(Data.NewValue);

	if (AbilitySystemComponent.IsValid())
	{
		float MaxVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxHealthAttribute());
		if (MaxVal > 0.f)
		{
			if (HealthBar) HealthBar->SetPercent(Data.NewValue / MaxVal);
		}
		if (HealthText) HealthText->SetText(FText::AsNumber((int32)Data.NewValue));
	}
}

void UBrawlHUDWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChangedDelegate.Broadcast(Data.NewValue);

	if (AbilitySystemComponent.IsValid())
	{
		float CurVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetHealthAttribute());
		if (Data.NewValue > 0.f)
		{
			if (HealthBar) HealthBar->SetPercent(CurVal / Data.NewValue);
		}
	}
}

void UBrawlHUDWidget::OnAmmoChanged(const FOnAttributeChangeData& Data)
{
	OnAmmoChangedDelegate.Broadcast(Data.NewValue);

	if (AbilitySystemComponent.IsValid())
	{
		float MaxVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxAmmoAttribute());
		
		// 슬롯 업데이트
		UpdateAmmoSlots(Data.NewValue, MaxVal);
	}
}

void UBrawlHUDWidget::OnMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
	OnMaxAmmoChangedDelegate.Broadcast(Data.NewValue);

	if (AbilitySystemComponent.IsValid())
	{
		float CurVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetAmmoAttribute());
		
		// 슬롯 업데이트
		UpdateAmmoSlots(CurVal, Data.NewValue);
	}
}

void UBrawlHUDWidget::UpdateAmmoSlots(float CurrentAmmo, float MaxAmmo)
{
	if (!AmmoSlotContainer || !AmmoSlotClass) return;

	int32 MaxAmmoInt = FMath::RoundToInt(MaxAmmo);
	if (MaxAmmoInt <= 0) return;

	// 1. 슬롯 개수 맞추기
	if (AmmoSlotWidgets.Num() != MaxAmmoInt)
	{
		AmmoSlotContainer->ClearChildren();
		AmmoSlotWidgets.Empty();

		for (int32 i = 0; i < MaxAmmoInt; i++)
		{
			UBrawlAmmoSlotWidget* SlotWidget = CreateWidget<UBrawlAmmoSlotWidget>(this, AmmoSlotClass);
			if (SlotWidget)
			{
				SlotWidget->InitSlot(i);
				AmmoSlotContainer->AddChild(SlotWidget);
				AmmoSlotWidgets.Add(SlotWidget);
			}
		}
	}

	// 2. 상태 업데이트
	int32 FullAmmoCount = FMath::FloorToInt(CurrentAmmo);
	
	// 어빌리티 타이머로부터 실제 재장전 진행도를 가져옴 (부드러운 연출)
	float PartialAmmo = GetReloadProgress();
	
	// 이미 탄약이 꽉 찼다면 진행도는 무시
	if (FullAmmoCount >= MaxAmmoInt)
	{
		PartialAmmo = 0.0f;
	}

	for (int32 i = 0; i < AmmoSlotWidgets.Num(); i++)
	{
		UBrawlAmmoSlotWidget* AmmoSlot = AmmoSlotWidgets[i];
		if (!AmmoSlot) continue;

		if (i < FullAmmoCount)
		{
			// 꽉 참
			AmmoSlot->UpdateState(true, false, 1.0f);
		}
		else if (i == FullAmmoCount)
		{
			// 충전 중 (가장 왼쪽의 비어있는 슬롯)
			AmmoSlot->UpdateState(false, true, PartialAmmo);
		}
		else
		{
			// 비어있음 (대기)
			AmmoSlot->UpdateState(false, false, 0.0f);
		}
	}
}

float UBrawlHUDWidget::GetReloadProgress() const
{
	if (!AbilitySystemComponent.IsValid()) return 0.0f;

	// 현재 캐릭터의 어빌리티 목록에서 Reload 어빌리티 인스턴스를 찾아 진행도를 가져옴
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (UBrawlGameplayAbility_Reload* ReloadAbility = Cast<UBrawlGameplayAbility_Reload>(Spec.GetPrimaryInstance()))
		{
			return ReloadAbility->GetReloadProgress();
		}
	}

	return 0.0f;
}

void UBrawlHUDWidget::PlayNoAmmoAnimation()
{
	for (UBrawlAmmoSlotWidget* AmmoSlot : AmmoSlotWidgets)
	{
		if (AmmoSlot)
		{
			AmmoSlot->PlayShakeAnimation();
		}
	}
}

void UBrawlHUDWidget::OnSuperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnSuperChargeChangedDelegate.Broadcast(Data.NewValue);

	if (SuperWidget && AbilitySystemComponent.IsValid())
	{
		float MaxVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxSuperChargeAttribute());
		if (MaxVal > 0.f)
		{
			SuperWidget->SetPercent(Data.NewValue / MaxVal);
			SuperWidget->SetIsReady(Data.NewValue >= MaxVal);
		}
	}
}

void UBrawlHUDWidget::OnMaxSuperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnMaxSuperChargeChangedDelegate.Broadcast(Data.NewValue);
	
	if (SuperWidget && AbilitySystemComponent.IsValid())
	{
		float CurVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetSuperChargeAttribute());
		if (Data.NewValue > 0.f)
		{
			SuperWidget->SetPercent(CurVal / Data.NewValue);
			SuperWidget->SetIsReady(CurVal >= Data.NewValue);
		}
	}
}

void UBrawlHUDWidget::OnHyperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnHyperChargeChangedDelegate.Broadcast(Data.NewValue);

	if (HyperWidget && AbilitySystemComponent.IsValid())
	{
		float MaxVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetMaxHyperChargeAttribute());
		if (MaxVal > 0.f)
		{
			HyperWidget->SetPercent(Data.NewValue / MaxVal);
			HyperWidget->SetIsReady(Data.NewValue >= MaxVal);
		}
	}
}

void UBrawlHUDWidget::OnMaxHyperChargeChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHyperChargeChangedDelegate.Broadcast(Data.NewValue);

	if (HyperWidget && AbilitySystemComponent.IsValid())
	{
		float CurVal = AbilitySystemComponent->GetNumericAttribute(UBrawlAttributeSet::GetHyperChargeAttribute());
		if (Data.NewValue > 0.f)
		{
			HyperWidget->SetPercent(CurVal / Data.NewValue);
			HyperWidget->SetIsReady(CurVal >= Data.NewValue);
		}
	}
}
void UBrawlHUDWidget::OnPowerCubeCountChanged(const FOnAttributeChangeData& Data)
{
	UpdatePowerCubeDisplay(Data.NewValue);
}

void UBrawlHUDWidget::UpdatePowerCubeDisplay(float NewCount)
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

void UBrawlHUDWidget::OnBountyChanged(int32 NewBounty)
{
	if (BountyText)
	{
		ESlateVisibility NewVisibility = (NewBounty > 1) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		
		BountyText->SetText(FText::AsNumber(NewBounty));
		BountyText->SetVisibility(NewVisibility);
		
		if (BountyIcon)
		{
			BountyIcon->SetVisibility(NewVisibility);
		}
	}
}

void UBrawlHUDWidget::OnTieBreakerStateChanged(bool bHasTieBreaker)
{
	if (TieBreakerIcon)
	{
		TieBreakerIcon->SetVisibility(bHasTieBreaker ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UBrawlHUDWidget::InitializeGameModeWidget()
{
	if (ActiveGameModeWidget) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState) return;

	TSubclassOf<AGameModeBase> CurrentGameModeClass = GameState->GameModeClass;
	if (!CurrentGameModeClass) return;

	TSubclassOf<UUserWidget> WidgetClassToSpawn = nullptr;

	for (const auto& Pair : GameModeSpecificWidgets)
	{
		if (Pair.Key && CurrentGameModeClass->IsChildOf(Pair.Key))
		{
			WidgetClassToSpawn = Pair.Value;
			break;
		}
	}

	if (WidgetClassToSpawn)
	{
		ActiveGameModeWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), WidgetClassToSpawn);
		if (ActiveGameModeWidget)
		{
			if (GameModeWidgetContainer)
			{
				GameModeWidgetContainer->AddChild(ActiveGameModeWidget);
			}
			else
			{
				ActiveGameModeWidget->AddToViewport();
			}
		}
	}
}

