// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "BrawlInputConfig.generated.h"

class UInputAction;

/**
 * FBrawlInputAction
 * 
 * 입력 액션과 GameplayTag를 매핑하는 구조체
 */
USTRUCT(BlueprintType)
struct FBrawlInputAction
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories = "Input"))
	TObjectPtr<const UInputAction> InputAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories = "Input"))
	FGameplayTag InputTag;
};

/**
 * UBrawlInputConfig
 * 
 * 프로젝트에서 사용하는 입력 설정을 담당하는 데이터 에셋
 * 네이티브 입력 (이동, 시선 등) 과 어빌리티 입력 (공격, 스킬 등) 을 구분하여 관리한다
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 기본 입력 액션 모음 (이동, 시선 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories = "InputAction"))
	TArray<FBrawlInputAction> NativeInputActions;
	
	// 어빌리티 입력 액션 모음 (공격, 스킬 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories = "InputAction"))
	TArray<FBrawlInputAction> AbilityInputActions;
	
public:
	// 네이티브 입력 액션 (코드에서 직접 바인딩)
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
	
	// 어빌리티 입력 액션 (GAS와 연동)
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
};
