// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlPawnComponent.h"
#include "BrawlHeroComponent.generated.h"

struct FGameplayTag;
class UBrawlInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/*
 * UBrawlHeroComponent
 * 
 * 플레이어 캐릭터(Hero)에 부착되어 입력 처리 및 카메라 제어 등
 * 플레이어 전용 로직을 담당하는 컴포넌트입니다.
 * Lyra의 ULyraHeroComponent를 참고하여 설계되었습니다.
 */
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BRAWLSTARSTPS_API UBrawlHeroComponent : public UBrawlPawnComponent
{
	GENERATED_BODY()
	
public:
	UBrawlHeroComponent(const FObjectInitializer& ObjectInitializer);
	
	// 컴포넌트 초기화 및 입력 시스템 설정
	void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	
	// 입력 바인딩 함수
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_Jump(const FInputActionValue& InputActionValue);
	void Input_StopJumping(const FInputActionValue& InputActionValue);
	
	// 어빌리티 입력 바인딩 함수
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
protected:
	// 기본 입력 매핑 컨텍스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brawl|Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
	
	// 액션 설정 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brawl|Input")
	TObjectPtr<UBrawlInputConfig> InputConfig;
	
	// 추후 제거/해제를 위한 어빌리티 입력 핸들 저장
	TArray<uint32> BindHandles;
};
