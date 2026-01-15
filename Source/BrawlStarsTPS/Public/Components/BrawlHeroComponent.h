// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrawlPawnComponent.h"
#include "BrawlHeroComponent.generated.h"

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
	
	// 추후 Ability Input 바인딩 함수 추가
	// void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	// void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
protected:
	// 기본 입력 매핑 컨텍스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brawl|Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
	
	// 이동 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brawl|Input")
	TObjectPtr<UInputAction> Input_MoveAction;
	
	// 시선 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brawl|Input")
	TObjectPtr<UInputAction> Input_LookAction;
};
