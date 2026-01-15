// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlCharacter.h"
#include "BrawlAbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlStarsTPS.h"

ABrawlCharacter::ABrawlCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UBrawlAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Attribute Set 생성
	AttributeSet = CreateDefaultSubobject<UBrawlAttributeSet>(TEXT("AttributeSet"));
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
	}
}

void ABrawlCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABrawlCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
