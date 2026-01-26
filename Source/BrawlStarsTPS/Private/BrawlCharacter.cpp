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
#include "Components/WidgetComponent.h"
#include "UI/BrawlHealthWidget.h"
#include "Kismet/GameplayStatics.h"

ABrawlCharacter::ABrawlCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
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
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f); // 카메라의 위치를 살짝 조정
	CameraBoom->bUsePawnControlRotation = true; // 스프링 암이 컨트롤러의 회전을 사용하도록 한다
	
	// 카메라 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 카메라는 스프링 암의 회전만 따라가면 됨

	// 체력바 위젯 컴포넌트
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(GetMesh());
	HealthBarComponent->SetWorldLocation(FVector(0.0f, 0.0f, 100.0f)); // 머리 위 높이
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

		// 머리 위 위젯 초기화
		if (HealthBarComponent)
		{
			HealthBarComponent->InitWidget(); // 위젯 인스턴스 확인 및 생성
			
			if (UUserWidget* WidgetObj = HealthBarComponent->GetUserWidgetObject())
			{
				if (UBrawlHealthWidget* HealthWidget = Cast<UBrawlHealthWidget>(WidgetObj))
				{
					UE_LOG(LogTemp, Warning, TEXT("BrawlCharacter::InitAbilityActorInfo - Initializing HealthWidget..."));
					HealthWidget->InitializeWithAbilitySystem(AbilitySystemComponent);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("BrawlCharacter::InitAbilityActorInfo - Widget Class is NOT UBrawlHealthWidget! Class: %s"), *WidgetObj->GetClass()->GetName());
				}
			}
			else
			{
				// 아직 위젯이 생성되지 않았을 수 있음 (비동기 등) -> 보통 InitWidget 후에는 있어야 함
				UE_LOG(LogTemp, Warning, TEXT("BrawlCharacter::InitAbilityActorInfo - GetUserWidgetObject returned NULL."));
			}
		}
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
		
		// 가젯 공격력
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetGadgetDamageAttribute(), Row->Gadget1Damage);
		
		// 가젯 쿨다운
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetGadgetCooldownAttribute(), Row->Gadget1Cooldown);
		
		// 궁극기 공격력
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetSuperDamageAttribute(), Row->SuperDamage);
		
		// 궁극기, 하이퍼차지 초기화
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
