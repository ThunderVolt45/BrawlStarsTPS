// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BrawlHeroComponent.h"

#include "GameFramework/Character.h"
#include "BrawlCharacter.h"
#include "BrawlAbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/BrawlInputComponent.h"
#include "Input/BrawlInputConfig.h"

UBrawlHeroComponent::UBrawlHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Tick이 필요하다면 true로 설정할 것
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


void UBrawlHeroComponent::OnRegister()
{
	Super::OnRegister();
	
	// 필요한 경우 GameFrameworkComponentManager에 등록 로직 추가
}

void UBrawlHeroComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBrawlHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	
	const APlayerController* PlayerController = Pawn->GetWorld()->GetFirstPlayerController();
	check(PlayerController);
	
	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	check(LocalPlayer);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);
	
	// 커스텀 입력 컨텍스트로 캐스팅
	UBrawlInputComponent* BrawlInputComponent = CastChecked<UBrawlInputComponent>(PlayerInputComponent);
	
	// 입력 컨텍스트 설정
	Subsystem->ClearAllMappings();
	if (DefaultInputMappingContext)
	{
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
	
	// 블루프린트 에디터(Project Settings)에 등록된 태그 이름과 정확히 일치해야 합니다.
	// 오타 방지를 위해 나중에 const 변수로 빼는 것을 고려해볼 수 있습니다.
	const FGameplayTag InputTag_Move = FGameplayTag::RequestGameplayTag(FName("InputTag.Move"));
	const FGameplayTag InputTag_Look = FGameplayTag::RequestGameplayTag(FName("InputTag.Look"));
	const FGameplayTag InputTag_Jump = FGameplayTag::RequestGameplayTag(FName("InputTag.Jump"));
	
	// 입력 액션 바인딩
	BrawlInputComponent->BindNativeAction(InputConfig, InputTag_Move, ETriggerEvent::Triggered, 
		this, &UBrawlHeroComponent::Input_Move, true);
	BrawlInputComponent->BindNativeAction(InputConfig, InputTag_Look, ETriggerEvent::Triggered,
		this, &UBrawlHeroComponent::Input_Look, true);
	BrawlInputComponent->BindNativeAction(InputConfig, InputTag_Jump, ETriggerEvent::Started,
		this, &UBrawlHeroComponent::Input_Jump, true);
	BrawlInputComponent->BindNativeAction(InputConfig, InputTag_Jump, ETriggerEvent::Completed,
		this, &UBrawlHeroComponent::Input_StopJumping, true);
	
	// 어빌리티 액션 바인딩
	BrawlInputComponent->BindAbilityAction(InputConfig, this,
		&UBrawlHeroComponent::Input_AbilityInputTagPressed, &UBrawlHeroComponent::Input_AbilityInputTagReleased, 
		BindHandles);
}

void UBrawlHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Pawn->GetControlRotation().Yaw, 0.0f);
	
	if (Value.X != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		 Pawn->AddMovementInput(MovementDirection, Value.X);
	}
	
	if (Value.Y != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		Pawn->AddMovementInput(MovementDirection, Value.Y);
	}
}

void UBrawlHeroComponent::Input_Look(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	
	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}
	
	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UBrawlHeroComponent::Input_Jump(const FInputActionValue& InputActionValue)
{
	if (ACharacter* Character = GetPawn<ACharacter>())
	{
		Character->Jump();
	}
}

void UBrawlHeroComponent::Input_StopJumping(const FInputActionValue& InputActionValue)
{
	if (ACharacter* Character = GetPawn<ACharacter>())
	{
		Character->StopJumping();
	}
}

void UBrawlHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const ABrawlCharacter* BrawlCharacter = Cast<ABrawlCharacter>(Pawn))
		{
			if (UBrawlAbilitySystemComponent* ASC = BrawlCharacter->GetBrawlAbilitySystemComponent())
			{
				ASC->AbilityInputTagPressed(InputTag);
			}
		}
	}
}

void UBrawlHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const ABrawlCharacter* BrawlCharacter = Cast<ABrawlCharacter>(Pawn))
		{
			if (UBrawlAbilitySystemComponent* ASC = BrawlCharacter->GetBrawlAbilitySystemComponent())
			{
				ASC->AbilityInputTagReleased(InputTag);
			}
		}
	}
}
