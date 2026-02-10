// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlCharacter.h"
#include "BrawlAbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlStarsTPS.h"
#include "BrawlStarsTPSGameMode.h"
#include "Data/BrawlCharacterData.h"
#include "Data/BrawlAIData.h"
#include "Camera/CameraComponent.h"
#include "Components/BrawlHeroComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/GameSession.h"
#include "UI/BrawlHealthWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Damage.h"
#include "GameplayEffect.h" // 추가
#include "Components/PostProcessComponent.h"
#include "Environment/BrawlPoisonZone.h"

ABrawlCharacter::ABrawlCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 독구름 화면 효과 컴포넌트 생성
	PoisonPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PoisonPostProcess"));
	PoisonPostProcess->SetupAttachment(RootComponent);
	PoisonPostProcess->bUnbound = false; // 기본은 꺼둠 (BeginPlay에서 로컬 플레이어만 켬)
	PoisonPostProcess->Priority = 10.0f;
	PoisonPostProcess->BlendWeight = 0.0f;
	PoisonPostProcess->bEnabled = false; // 기본 비활성화
	
	// 캐릭터 몸체의 회전이 컨트롤러의 Yaw 값을 따라가도록 한다
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
	// 캐릭터가 이동 방향으로 자동으로 회전하지 않도록 한다
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->JumpZVelocity = 500.0f;
	
	// 스프링 암 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	
	// 소켓 오프셋: 카메라를 캐릭터의 오른쪽(Y), 위쪽(Z)으로 이동
	// 이렇게 하면 카메라는 캐릭터의 우측 상단에서 전방을 바라보게 되어, 
	// 화면상에서 캐릭터는 좌측 하단에 위치하게 됩니다. (버블파이터/TPS 뷰)
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f); 
	
	// 타겟 오프셋 제거 (회전 축은 캐릭터 중심 유지)
	CameraBoom->TargetOffset = FVector::ZeroVector;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bInheritRoll = false; // Roll 상속 차단
	
	// 카메라 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 카메라 자체 회전은 0으로 리셋 (기울어짐의 주원인 제거)
	FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);

	// 체력바 위젯 컴포넌트
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(GetMesh());
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // 머리 위 높이 (상대 좌표)
	HealthBarComponent->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::World);
	HealthBarComponent->SetDrawSize(FVector2D(200.0f, 50.0f));
	HealthBarComponent->SetOwnerNoSee(true); // 본인에게는 보이지 않도록 설정
	HealthBarComponent->SetCastShadow(false); // 그림자 생성 안 함
	HealthBarComponent->SetReceivesDecals(false); // 데칼 무시

	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UBrawlAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Attribute Set 생성
	AttributeSet = CreateDefaultSubobject<UBrawlAttributeSet>(TEXT("AttributeSet"));
	
	// Hero 컴포넌트 생성
	HeroComponent = CreateDefaultSubobject<UBrawlHeroComponent>(TEXT("HeroComponent"));
}

UBehaviorTree* ABrawlCharacter::GetCombatBehaviorTree() const
{
	if (!AIDataTable)
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("Get Combat BT"));
	FBrawlAIData* Row = AIDataTable->FindRow<FBrawlAIData>(CharacterID, ContextString);

	if (Row)
	{
		return Row->CombatBehaviorTree;
	}

	return nullptr;
}

FBrawlCharacterData ABrawlCharacter::GetCharacterData() const
{
	if (CharacterDataTable)
	{
		static const FString ContextString(TEXT("Get Character Data"));
		if (FBrawlCharacterData* Row = CharacterDataTable->FindRow<FBrawlCharacterData>(CharacterID, ContextString))
		{
			return *Row;
		}
	}
	return FBrawlCharacterData();
}

UTexture2D* ABrawlCharacter::GetCharacterIcon() const
{
	FBrawlCharacterData Data = GetCharacterData();
	
	if (Data.CharacterIcon.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetCharacterIcon: Icon SoftPtr is NULL for Character [%s]"), *CharacterID.ToString());
		return nullptr;
	}

	UTexture2D* LoadedIcon = Data.CharacterIcon.LoadSynchronous();
	if (!LoadedIcon)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetCharacterIcon: Failed to LoadSynchronous Icon for [%s]. Path: %s"), 
			*CharacterID.ToString(), *Data.CharacterIcon.ToString());
	}
	
	return LoadedIcon;
}

void ABrawlCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 독구름 화면 효과 머티리얼 설정 (로컬 플레이어만)
	if (IsLocallyControlled())
	{
		if (PoisonPostProcess)
		{
			PoisonPostProcess->bUnbound = true;
			PoisonPostProcess->bEnabled = true;
		}

		if (PoisonPPMaterial)
		{
			PoisonPPMaterialInstance = UMaterialInstanceDynamic::Create(PoisonPPMaterial, this);
			if (PoisonPPMaterialInstance && PoisonPostProcess)
			{
				PoisonPostProcess->AddOrUpdateBlendable(PoisonPPMaterialInstance, 1.0f);
			}
		}
	}
}

UAbilitySystemComponent* ABrawlCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABrawlCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID.GetId();
}

FGenericTeamId ABrawlCharacter::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}

void ABrawlCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 서버에서 GAS 초기화
	InitAbilityActorInfo();

	// 속성 초기화
	InitializeAttributes();

	// 기본 어빌리티 부여
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddCharacterAbilities(StartupAbilities);
	}
}

void ABrawlCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서 GAS 초기화
	InitAbilityActorInfo();
}

void ABrawlCharacter::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlCharacter: AbilitySystemComponent is NOT EXIST!!"));
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// 이동 속도 변화 감지 바인딩
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UBrawlAttributeSet::GetMovementSpeedAttribute()).AddUObject(this, &ABrawlCharacter::OnMovementSpeedChanged);

	// 체력 변화 감지 바인딩
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UBrawlAttributeSet::GetHealthAttribute()).AddUObject(this, &ABrawlCharacter::OnHealthChanged);

	// 전투 상태 태그(State.Combat) 변화 감지 바인딩 (공격 중일 때 즉시 노출)
	CombatStateTagDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(CombatTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ABrawlCharacter::OnCombatRevealedTagChanged);
	
	// 발각 태그(State.Combat.Revealed) 변화 감지 바인딩
	CombatRevealedTagDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(RevealedTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ABrawlCharacter::OnCombatRevealedTagChanged);

	// 하이퍼차지 태그(State.Hypercharged) 변화 감지 바인딩
	HyperChargeTagDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(HyperChargeTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ABrawlCharacter::OnHyperChargeTagChanged);

	// 리스폰 GameplayCue 호출
	if (RespawnCueTag.IsValid())
	{
		AbilitySystemComponent->ExecuteGameplayCue(RespawnCueTag);
	}

	// 머리 위 위젯 초기화
	if (HealthBarComponent)
	{
		HealthBarComponent->InitWidget(); // 위젯 인스턴스 확인 및 생성

		if (UUserWidget* WidgetObj = HealthBarComponent->GetUserWidgetObject())
		{
			if (UBrawlHealthWidget* HealthWidget = Cast<UBrawlHealthWidget>(WidgetObj))
			{
				// UE_LOG(LogTemp, Warning, TEXT("BrawlCharacter::InitAbilityActorInfo - Initializing HealthWidget..."));
				HealthWidget->InitializeWithAbilitySystem(AbilitySystemComponent);
			}
			else
			{
				UE_LOG(LogTemp, Error,
				       TEXT("BrawlCharacter::InitAbilityActorInfo - Widget Class is NOT UBrawlHealthWidget! Class: %s"),
				       *WidgetObj->GetClass()->GetName());
			}
		}
		else
		{
			// 아직 위젯이 생성되지 않았을 수 있음 (비동기 등) -> 보통 InitWidget 후에는 있어야 함
			UE_LOG(LogTemp, Warning, TEXT("BrawlCharacter::InitAbilityActorInfo - GetUserWidgetObject returned NULL."));
		}
	}
}

void ABrawlCharacter::OnMovementSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
		
		// UE_LOG(LogTemp, Warning, TEXT("BrawlCharacter::OnMovementSpeedChanged - Speed Updated: %.2f"), Data.NewValue);
	}
}

void ABrawlCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 플레이어 제어 시 메시 회전 보정 (숄더뷰에서 캐릭터가 중앙을 바라보는 느낌을 주기 위함)
	if (IsLocallyControlled() && GetMesh())
	{
		FRotator CurrentMeshRot = GetMesh()->GetRelativeRotation();
		// 캐릭터 메시의 기본 방향(-90)에 사용자 설정 오프셋을 더함
		float TargetYaw = -90.0f + ControlledMeshYawOffset;
		
		if (!FMath::IsNearlyEqual(CurrentMeshRot.Yaw, TargetYaw, 0.1f))
		{
			// 부드럽게 회전 보정
			float NewYaw = FMath::FInterpTo(CurrentMeshRot.Yaw, TargetYaw, DeltaTime, 10.0f);
			GetMesh()->SetRelativeRotation(FRotator(CurrentMeshRot.Pitch, NewYaw, CurrentMeshRot.Roll));
		}
	}

	// 독구름 화면 효과 업데이트 (로컬 플레이어만)
	if (IsLocallyControlled())
	{
		UpdatePoisonScreenEffect(DeltaTime);
	}
	
	// 체력바 빌보드 처리 (카메라 방향을 보게 함)
	if (HealthBarComponent && HealthBarComponent->GetWidgetSpace() == EWidgetSpace::World)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			FVector CameraLocation;
			FRotator CameraRotation;
			PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
			
			// 카메라의 전방 벡터를 가져와 반대로 뒤집는다
			FVector CameraForward = CameraRotation.Vector() * -1.0f;
			FRotator CameraRotator = CameraForward.Rotation();
			
			// UI가 기울어지지 않고 카메라와 마주볼 수 있다
			HealthBarComponent->SetWorldRotation(CameraRotator);
		}
	}
}

void ABrawlCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Hero 컴포넌트를 통해 입력 바인딩 초기화
	if (HeroComponent)
	{
		HeroComponent->InitializePlayerInput(PlayerInputComponent);
	}
}

void ABrawlCharacter::SetInBush(bool bInBush)
{
	if (bInBush)
	{
		BushOverlapCount++;
	}
	else
	{
		BushOverlapCount--;
	}

	// 카운트가 0 이하로 떨어지지 않도록 보정
	if (BushOverlapCount < 0)
	{
		BushOverlapCount = 0;
	}

	// 수풀에 하나라도 겹쳐 있으면 숨김 상태
	bool bNewHiddenState = (BushOverlapCount > 0);

	if (bIsHiddenInBush != bNewHiddenState)
	{
		bIsHiddenInBush = bNewHiddenState;
		UpdateMeshVisibility();
		
		// UE_LOG(LogTemp, Log, TEXT("Character [%s] Hidden State Changed: %s"), *GetName(), 
		// 	bIsHiddenInBush ? TEXT("HIDDEN") : TEXT("VISIBLE"));
	}
}

void ABrawlCharacter::SetRevealed(bool bRevealed)
{
	if (bIsRevealed != bRevealed)
	{
		bIsRevealed = bRevealed;
		UpdateMeshVisibility();
		
		// UE_LOG(LogTemp, Log, TEXT("Character [%s] Revealed State Changed: %s"), *GetName(), 
		// 	bIsRevealed ? TEXT("REVEALED") : TEXT("HIDDEN"));
	}
}

void ABrawlCharacter::NotifyCombatAction()
{
	ApplyCombatRevealEffect();
}

void ABrawlCharacter::NotifyHyperChargeActivated()
{
	// 태그 기반 시스템(OnHyperChargeTagChanged)으로 이관됨
	// 필요 시 즉발성 SFX 등은 여기서 처리 가능
}

void ABrawlCharacter::ApplyCombatRevealEffect()
{
	if (!AbilitySystemComponent) return;

	if (CombatRevealEffectClass)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		
		// 레벨은 1.0f로 적용 (필요 시 변경 가능)
		AbilitySystemComponent->ApplyGameplayEffectToSelf(
			CombatRevealEffectClass.GetDefaultObject(), 
			1.0f, 
			Context
		);
		
		// UE_LOG(LogTemp, Log, TEXT("Character [%s] Applied Combat Reveal Effect (BP Class)."), *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Character [%s] has NO CombatRevealEffectClass set! Combat visibility will NOT work."), *GetName());
	}
}

void ABrawlCharacter::OnCombatRevealedTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (CallbackTag == RevealedTag)
	{
		bIsRevealedByCombat = (NewCount > 0);
	}
	else if (CallbackTag == CombatTag)
	{
		bIsCombatState = (NewCount > 0);
	}

	UpdateMeshVisibility();
}

void ABrawlCharacter::OnHyperChargeTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		// 하이퍼차지 시작: 액터 스폰
		if (HyperChargeEffectClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = this;

			// 위치는 발 밑에서 시작
			FVector SpawnLocation = GetActorLocation();
			if (GetCapsuleComponent())
			{
				SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			}

			HyperChargeEffectInstance = GetWorld()->SpawnActor<AActor>(HyperChargeEffectClass, SpawnLocation, GetActorRotation(), SpawnParams);

			if (HyperChargeEffectInstance)
			{
				// 메시의 Root에 부착 (블루프린트에서 위치/회전 조정 용이하도록)
				HyperChargeEffectInstance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			}
		}
	}
	else
	{
		// 하이퍼차지 종료 상태
		// 직접 제거하는 것도 가능하지만 일단 하이퍼차지 효과 블루프린트가 스스로 정리하게 둔다
	}
}

bool ABrawlCharacter::IsVisibleTo(const FGenericTeamId& ObserverTeam) const
{
	// 1. 수풀에 없다면 항상 보임
	if (!bIsHiddenInBush)
	{
		return true;
	}

	// 2. 같은 팀에게는 항상 보임 (옵저버도 포함)
	FGenericTeamId MyTeam = GetGenericTeamId();
	if (MyTeam != FGenericTeamId::NoTeam && MyTeam == ObserverTeam)
	{
		return true;
	}

	// 3. 발각된 상태라면(근처에 적이 있음) 적에게도 보임
	if (bIsRevealed)
	{
		return true;
	}

	// 4. 전투 상태 태그가 있다면 보임
	// - State.Combat.Revealed: 피격 등으로 인해 일정 시간 노출
	// - State.Combat: 공격 행동 중이라 노출
	if (bIsRevealedByCombat || bIsCombatState)
	{
		return true;
	}

	// 5. 그 외의 경우(수풀 속 + 발각 안됨 + 전투 안함 + 적군) -> 안 보임
	return false;
}

void ABrawlCharacter::UpdateMeshVisibility()
{
	// 최종 은신 여부 판별
	// 수풀에 있고(HiddenInBush) 발각되지 않은 상태(Revealed) 인가?
	// 그리고 전투 중(CombatState)이거나 피격 노출(RevealedByCombat) 상태가 아니어야 함
	bool bFinalHidden = bIsHiddenInBush && !bIsRevealed && !bIsRevealedByCombat && !bIsCombatState;

	// 로컬 플레이어는 항상 보여야 함
	if (IsPlayerControlled())
	{
		if (GetMesh())
		{
			GetMesh()->SetHiddenInGame(false);
		}
	}
	// 적(AI)은 조건 만족 시 완전 투명(HiddenInGame) 처리
	else
	{
		// 다른 캐릭터(적/AI): 은신 조건 만족 시 메시를 아예 숨김
		if (GetMesh())
		{
			// 자신과 모든 자식 컴포넌트(무기 등)를 포함하여 숨김 처리
			GetMesh()->SetHiddenInGame(bFinalHidden, true);
		}
		
		// 체력바 등 부착된 위젯도 같이 숨김 처리
		if (HealthBarComponent)
		{
			HealthBarComponent->SetHiddenInGame(bFinalHidden);
		}
	}
}

void ABrawlCharacter::InitializeAttributes()
{
	if (!AbilitySystemComponent || !CharacterDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeAttributes Failed! ASC or DT is NULL"));
		return;
	}

	// 1. 기본 스탯 데이터 로드 (DT_BrawlerData)
	static const FString ContextString(TEXT("Init Attributes"));
	FBrawlCharacterData* Row = CharacterDataTable->FindRow<FBrawlCharacterData>(CharacterID, ContextString);
	
	if (Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("BrawlCharacter::InitializeAttributes - Loaded Data for [%s]. MaxHealth: %f, MaxAmmo: %f, MovementSpeed: %f"), 
			*CharacterID.ToString(), Row->MaxHealth, Row->MaxAmmo, Row->MoveSpeed);

		// GE를 사용하지 않고 직접 Base Value 설정 (안전하고 확실함)
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxHealthAttribute(), Row->MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHealthAttribute(), Row->MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxAmmoAttribute(), Row->MaxAmmo);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetAmmoAttribute(), Row->MaxAmmo);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetReloadSpeedAttribute(), Row->ReloadDelay);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMovementSpeedAttribute(), Row->MoveSpeed);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetAttackDamageAttribute(), Row->AttackDamage);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetGadgetDamageAttribute(), Row->Gadget1Damage);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetGadgetCooldownAttribute(), Row->Gadget1Cooldown);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetGadget2DamageAttribute(), Row->Gadget2Damage);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetGadget2CooldownAttribute(), Row->Gadget2Cooldown);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetSuperDamageAttribute(), Row->SuperDamage);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxSuperChargeAttribute(), Row->MaxSuperCharge);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetSuperChargeAttribute(), 0.0f);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetSuperCostAttribute(), Row->SuperCost);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetSuperChargePerHitAttribute(), Row->SuperChargePerHit);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxHyperChargeAttribute(), Row->MaxHyperCharge);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHyperChargeAttribute(), 0.0f);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHyperChargePerHitAttribute(), Row->HyperChargePerHit);
		
		// 이동 속도 값은 CharacterMovement에 직접 주입한다
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = Row->MoveSpeed;
		}
	}

	// 2. AI 설정 데이터 로드 (DT_BrawlerAI)
	if (AIDataTable)
	{
		FBrawlAIData* AIRow = AIDataTable->FindRow<FBrawlAIData>(CharacterID, ContextString);
		if (AIRow)
		{
			AICombatSettings.MaxCombatRange = AIRow->MaxCombatRange;
			AICombatSettings.PreferredCombatRange = AIRow->PreferredCombatRange;
			AICombatSettings.MinCombatRange = AIRow->MinCombatRange;
			AICombatSettings.FleeHealthRatio = AIRow->FleeHealthRatio;
			AICombatSettings.ResumeCombatHealthRatio = AIRow->ResumeCombatHealthRatio;
			AICombatSettings.PursuitTargetHealthRatio = AIRow->PursuitTargetHealthRatio;
			
			UE_LOG(LogTemp, Warning, TEXT("AI Data Loaded for [%s]"), *CharacterID.ToString());
		}
	}
}

void ABrawlCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 이미 사망했으면 무시
	if (bIsDead) return;

	// 데미지를 입은 경우 (체력 감소)
	if (Data.NewValue < Data.OldValue)
	{
		NotifyCombatAction();
	}

	// 체력이 0 이하면 사망 처리
	if (Data.NewValue <= 0.0f)
	{
		Die();
	}
}

void ABrawlCharacter::Die()
{
	if (bIsDead) return;

	bIsDead = true;

	// 사망 GameplayCue 호출
	if (AbilitySystemComponent && DeathCueTag.IsValid())
	{
		AbilitySystemComponent->ExecuteGameplayCue(DeathCueTag);
	}

	// 서버에서 GameMode에 사망 사실 알림 (GameState를 통해 클라이언트로 전파됨)
	if (HasAuthority())
	{
		if (ABrawlStarsTPSGameMode* GM = GetWorld()->GetAuthGameMode<ABrawlStarsTPSGameMode>())
		{
			GM->NotifyKill(LastHitInstigator, this);
		}
	}

	// 1. 컨트롤러 분리 (입력 차단)
	AController* OldController = GetController();
	if (OldController)
	{
		DetachFromControllerPendingDestroy();
	}

	// 2. 캡슐 콜리전 비활성화 (이동 불가 및 물리 간섭 제거)
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 3. 캐릭터 무브먼트 비활성화
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetComponentTickEnabled(false);
	}

	// 5. 체력바 숨김
	if (HealthBarComponent)
	{
		HealthBarComponent->SetHiddenInGame(true);
	}
	
	// 6. Actor 제거 (혹은 리스폰 로직으로 대체 가능)
	Destroy();
}

void ABrawlCharacter::UpdatePoisonScreenEffect(float DeltaTime)
{
	if (!PoisonPostProcess) return;
	
	bool bInPoison = false;
	
	// 독구름 액터 캐싱 처리
	if (!CachedPoisonZone)
	{
		CachedPoisonZone = Cast<ABrawlPoisonZone>(UGameplayStatics::GetActorOfClass(GetWorld(), ABrawlPoisonZone::StaticClass()));
	}
	
	if (CachedPoisonZone)
	{
		// 안전 구역 밖인지 체크
		if (!CachedPoisonZone->IsPositionSafe(GetActorLocation()))
		{
			bInPoison = true;
		}
	}
	
	// 목표 강도 설정
	float TargetIntensity = bInPoison ? 1.0f : 0.0f;
	
	// 보간 속도 차등 적용: 들어갈 때는 빠르게(8.0), 나올 때는 부드럽게(3.0)
	float InterpSpeed = (TargetIntensity > CurrentPoisonIntensity) ? 8.0f : 3.0f;
	CurrentPoisonIntensity = FMath::FInterpTo(CurrentPoisonIntensity, TargetIntensity, DeltaTime, InterpSpeed);

	// 포스트 프로세스 가중치 조절
	PoisonPostProcess->BlendWeight = CurrentPoisonIntensity;
	
	// 머티리얼 파라미터 업데이트
	if (PoisonPPMaterialInstance)
	{
		PoisonPPMaterialInstance->SetScalarParameterValue(TEXT("Intensity"), CurrentPoisonIntensity);
	}
}
