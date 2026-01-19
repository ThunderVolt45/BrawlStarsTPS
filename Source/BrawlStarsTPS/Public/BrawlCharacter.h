// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "BrawlCharacter.generated.h"

class USpringArmComponent;
class UBrawlAbilitySystemComponent;
class UBrawlAttributeSet;
class UBrawlHeroComponent;

/**
 * ABrawlCharacter
 *
 * Lyra 스타일의 모듈형 아키텍처를 지향하는 프로젝트의 기본 캐릭터 클래스입니다.
 * GAS(Gameplay Ability System)를 기본적으로 지원하며, 
 * 입력 및 카메라 로직은 추후 HeroComponent 등으로 분리될 예정입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABrawlCharacter();

	//~IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	UFUNCTION(BlueprintCallable, Category = "Brawl|Character")
	UBrawlAbilitySystemComponent* GetBrawlAbilitySystemComponent() const { return AbilitySystemComponent; }

protected:
	virtual void BeginPlay() override;

	/** GAS 초기화 */
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** 캐릭터 기본 설정 (GAS 관련) */
	void InitAbilityActorInfo();

protected:
	// 어빌리티 시스템 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBrawlAbilitySystemComponent> AbilitySystemComponent;

	// 어트리뷰트 세트
	UPROPERTY()
	TObjectPtr<const UBrawlAttributeSet> AttributeSet;
	
	// 플레이어 전용 로직 및 입력을 담당하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Character")
	TObjectPtr<UBrawlHeroComponent> HeroComponent;
	
	// 스프링 암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Camera")
	TObjectPtr<class UCameraComponent> FollowCamera;

public:	
	FORCEINLINE USpringArmComponent* GetCameraBoom() { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() { return FollowCamera; }
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
