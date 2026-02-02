// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlPowerCubeBox.h"
#include "BrawlAbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "GameplayEffectExtension.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Environment/BrawlPowerCube.h" // 추가

ABrawlPowerCubeBox::ABrawlPowerCubeBox()
{
	PrimaryActorTick.bCanEverTick = false;

	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UBrawlAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// 어트리뷰트 세트 생성
	AttributeSet = CreateDefaultSubobject<UBrawlAttributeSet>(TEXT("AttributeSet"));

	// 체력바 컴포넌트
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(FVector2D(100.0f, 20.0f));
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f)); // 박스 위로 띄움

	// AI 감지 소스 (시각)
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
	StimuliSourceComponent->bAutoRegister = true;
	StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
	
	// ABrawlObstacle 설정
	bIsDestructible = true; // OnDestruction 로직 수행을 위해 true로 유지 (IsDestructible() 함수 오버라이드로 외부에는 false로 속임)
	bIsHardObstacle = true;
}

UAbilitySystemComponent* ABrawlPowerCubeBox::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABrawlPowerCubeBox::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID.GetId();
}

FGenericTeamId ABrawlPowerCubeBox::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}

bool ABrawlPowerCubeBox::IsDestructible() const
{
	// 발사체 등 외부 시스템에는 "파괴 불가능"으로 보이게 하여
	// 즉사(Instant Kill)를 방지하고 GAS 데미지 시스템을 따르게 함
	return false;
}

void ABrawlPowerCubeBox::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 어트리뷰트 변경 콜백 등록
		if (AttributeSet)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &ABrawlPowerCubeBox::OnHealthChanged);
		}
	}
}

void ABrawlPowerCubeBox::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent && AttributeSet)
	{
		// 기본 체력 설정 (GameplayEffect 이용 또는 직접 설정)
		// 여기서는 간단히 직접 설정 (서버 권한 필요)
		if (HasAuthority())
		{
			// MaxHealth 설정
			AbilitySystemComponent->ApplyModToAttributeUnsafe(AttributeSet->GetMaxHealthAttribute(), EGameplayModOp::Override, DefaultMaxHealth);
			// Health 설정
			AbilitySystemComponent->ApplyModToAttributeUnsafe(AttributeSet->GetHealthAttribute(), EGameplayModOp::Override, DefaultMaxHealth);
		}
	}
}

void ABrawlPowerCubeBox::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	float NewHealth = Data.NewValue;

	if (NewHealth <= 0.0f && !bIsDead)
	{
		AActor* Killer = nullptr;
		// Data.EffectSpec.GetEffectContext().GetInstigator(); // 필요 시 가져옴
		
		Die(Killer);
	}
}

void ABrawlPowerCubeBox::Die(AActor* Killer)
{
	if (bIsDead) return;
	bIsDead = true;

	// 파워 큐브 드롭
	if (PowerCubeClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// 살짝 위에서 스폰
		FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		GetWorld()->SpawnActor<ABrawlPowerCube>(PowerCubeClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	// 부모 클래스의 파괴 로직(파편 생성, 사운드 등) 실행
	Super::OnDestruction(Killer);
}
