// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlCharacter.h"
#include "BrawlAbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlStarsTPS.h"
#include "Data/BrawlCharacterData.h"
#include "Camera/CameraComponent.h"
#include "Components/BrawlHeroComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ABrawlCharacter::ABrawlCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 캐릭터 몸체의 회전이 컨트롤러의 Yaw 값을 따라가도록 한다
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
	// 캐릭터가 이동 방향으로 자동으로 회전하지 않도록 한다
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	// 스프링 암 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f); // 카메라의 위치를 살짝 조정
	CameraBoom->bUsePawnControlRotation = true; // 스프링 암이 컨트롤러의 회전을 사용하도록 한다
	
	// 카메라 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 카메라는 스프링 암의 회전만 따라가면 됨

	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UBrawlAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Attribute Set 생성
	AttributeSet = CreateDefaultSubobject<UBrawlAttributeSet>(TEXT("AttributeSet"));
	
	// Hero 컴포넌트 생성
	HeroComponent = CreateDefaultSubobject<UBrawlHeroComponent>(TEXT("HeroComponent"));
}

void ABrawlCharacter::BeginPlay()
{
	Super::BeginPlay();
}

UAbilitySystemComponent* ABrawlCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 이동 속도 변화 감지 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBrawlAttributeSet::GetMovementSpeedAttribute()).AddUObject(this, &ABrawlCharacter::OnMovementSpeedChanged);
	}
}

void ABrawlCharacter::OnMovementSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
		
		UE_LOG(LogTemp, Warning, TEXT("BrawlCharacter::OnMovementSpeedChanged - Speed Updated: %.2f"), Data.NewValue);
	}
}

void ABrawlCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

void ABrawlCharacter::InitializeAttributes()
{
	if (!AbilitySystemComponent || !CharacterDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeAttributes Failed! ASC or DT is NULL"));
		return;
	}

	// 데이터 테이블에서 Row 찾기
	static const FString ContextString(TEXT("Init Attributes"));
	FBrawlCharacterData* Row = CharacterDataTable->FindRow<FBrawlCharacterData>(CharacterID, ContextString);
	
	if (Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("BrawlCharacter::InitializeAttributes - Loaded Data for [%s]. MaxHealth: %f, MaxAmmo: %f, MovementSpeed: %f"), 
			*CharacterID.ToString(), Row->MaxHealth, Row->MaxAmmo, Row->MoveSpeed);

		// GE를 사용하지 않고 직접 Base Value 설정 (안전하고 확실함)
		// 체력
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxHealthAttribute(), Row->MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHealthAttribute(), Row->MaxHealth);

		// 탄환
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxAmmoAttribute(), Row->MaxAmmo);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetAmmoAttribute(), Row->MaxAmmo);
		
		// 재장전
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetReloadSpeedAttribute(), Row->ReloadDelay);
		
		// 이동 속도
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMovementSpeedAttribute(), Row->MoveSpeed);
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = Row->MoveSpeed;
		}

		// 공격력
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetAttackDamageAttribute(), Row->AttackDamage);
		
		// 게이지 초기화
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxSuperChargeAttribute(), Row->MaxSuperCharge);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetSuperChargeAttribute(), 0.0f);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetSuperCostAttribute(), Row->SuperCost);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetSuperChargePerHitAttribute(), Row->SuperChargePerHit);

		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxHyperChargeAttribute(), Row->MaxHyperCharge);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHyperChargeAttribute(), 0.0f);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHyperChargePerHitAttribute(), Row->HyperChargePerHit);

		
		UE_LOG(LogTemp, Warning, TEXT("Attributes Initialized via C++ Direct Set."));
	}
}
