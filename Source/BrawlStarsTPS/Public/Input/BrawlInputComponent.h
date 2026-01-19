// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/BrawlInputConfig.h"
#include "EnhancedInputComponent.h"
#include "BrawlInputComponent.generated.h"

/**
 * UBrawlInputComponent
 * 
 * EnhancedInputComponent를 확장하여 GameplayTag를 기반으로 입력을 바인딩할 수 있도록 합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	UBrawlInputComponent(const FObjectInitializer& ObjectInitializer);
	
	// 네이티브 액션 바인딩 헬퍼 함수
	template<class UserClass, typename FuncType>
	void BindNativeAction(const UBrawlInputConfig* InputConfig, const FGameplayTag& InputTag, 
		ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);
	
	// 어빌리티 액션 바인딩 헬퍼 함수
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityAction(const UBrawlInputConfig* InputConfig, UserClass* Object, 
		PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);
};

template <class UserClass, typename FuncType>
void UBrawlInputComponent::BindNativeAction(const UBrawlInputConfig* InputConfig, const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UBrawlInputComponent::BindAbilityAction(const UBrawlInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);
	
	for (const FBrawlInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, 
					Object, PressedFunc, Action.InputTag).GetHandle());
			}
			
			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed,
					Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}
