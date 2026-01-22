// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "BrawlCharacter.generated.h"

struct FOnAttributeChangeData;
class UGameplayEffect;
class UCameraComponent;
class USpringArmComponent;
class UBrawlAbilitySystemComponent;
class UBrawlAttributeSet;
class UBrawlHeroComponent;
class UBrawlGameplayAbility;

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

	// GAS 초기화
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** 캐릭터 기본 설정 (GAS 관련) */
	void InitAbilityActorInfo();

	/** 데이터 테이블을 이용한 속성 초기화 */
	void InitializeAttributes();

protected:
	// 캐릭터 ID (데이터 테이블의 Row Name과 일치해야 함)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	FName CharacterID = FName("Colt");

	// 능력치 데이터 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Stats")
	TObjectPtr<UDataTable> CharacterDataTable;

	// 초기화용 Gameplay Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Stats")
	TSubclassOf<UGameplayEffect> InitStatsEffectClass;

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

	// 게임 시작 시 부여할 기본 어빌리티 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Abilities")
	TArray<TSubclassOf<UBrawlGameplayAbility>> StartupAbilities;
	
	// 스프링 암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

public:	
	FORCEINLINE USpringArmComponent* GetCameraBoom() { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() { return FollowCamera; }
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// 이동 속도 속성 변경 시 호출될 콜백
	void OnMovementSpeedChanged(const FOnAttributeChangeData& Data);

};
