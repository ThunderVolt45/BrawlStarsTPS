// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BrawlHeroComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

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
	
	// 매핑 컨텍스트 초기화
	Subsystem->ClearAllMappings();
	
	if (DefaultInputMappingContext)
	{
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
	
	// 입력 액션 바인딩
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	
	if (Input_MoveAction)
	{
		EnhancedInputComponent->BindAction(Input_MoveAction, ETriggerEvent::Triggered, 
			this, &UBrawlHeroComponent::Input_Move);
	}
	
	if (Input_LookAction)
	{
		EnhancedInputComponent->BindAction(Input_LookAction, ETriggerEvent::Triggered,
			this, &UBrawlHeroComponent::Input_Look);
	}
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
