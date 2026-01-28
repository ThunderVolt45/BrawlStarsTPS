// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "BrawlCharacter.generated.h"

struct FOnAttributeChangeData;
class UGameplayEffect;
class UCameraComponent;
class USpringArmComponent;
class UBrawlAbilitySystemComponent;
class UBrawlAttributeSet;
class UBrawlHeroComponent;
class UBrawlGameplayAbility;
class UWidgetComponent;

/**
 * AI 전투 설정 (브롤러별 거리 및 체력 기준)
 */
USTRUCT(BlueprintType)
struct FAICombatSettings
{
	GENERATED_BODY()

	// 최대 교전 거리 (이보다 멀면 "이동" 전략)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxCombatRange = 1000.0f;

	// 선호 교전 거리 (이 거리 유지를 위해 이동/후퇴)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PreferredCombatRange = 700.0f;

	// 최소 교전 거리 (이보다 가까우면 "도주" 전략)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MinCombatRange = 300.0f;

	// 도주 시작 체력 비율 (0.0 ~ 1.0) - 이 이하로 떨어지면 도주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float FleeHealthRatio = 0.3f;

	// 도주 종료(복귀) 체력 비율 (0.0 ~ 1.0) - 이 이상 회복되면 다시 교전
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ResumeCombatHealthRatio = 0.7f;
};

/**
 * ABrawlCharacter
 *
 * Lyra 스타일의 모듈형 아키텍처를 지향하는 프로젝트의 기본 캐릭터 클래스입니다.
 * GAS(Gameplay Ability System)를 사용합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlCharacter : public ACharacter, 
	public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ABrawlCharacter();

	//~IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	//~IGenericTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~End of IGenericTeamAgentInterface interface

	UFUNCTION(BlueprintCallable, Category = "Brawl|Character")
	UBrawlAbilitySystemComponent* GetBrawlAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Character")
	int32 GetTeamID() const { return TeamID; }

	// AI 설정 반환
	UFUNCTION(BlueprintCallable, Category = "Brawl|AI")
	const FAICombatSettings& GetAICombatSettings() const { return AICombatSettings; }

protected:
	virtual void BeginPlay() override;

	// GAS 초기화
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// 캐릭터 기본 설정 (GAS 관련)
	void InitAbilityActorInfo();

	// 데이터 테이블을 이용한 속성 초기화
	void InitializeAttributes();

protected:
	// 머리 위 체력바 위젯 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	// 캐릭터 ID (데이터 테이블의 Row Name과 일치해야 함)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	FName CharacterID = FName("Colt");

	// 팀 ID (0: 레드팀, 1: 블루팀, 255: 중립 / 팀 없음 (모두 적대))
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	uint8 TeamID = 255;

	// 능력치 데이터 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Stats")
	TObjectPtr<UDataTable> CharacterDataTable;

	// 초기화용 Gameplay Effect 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Stats")
	TSubclassOf<UGameplayEffect> InitStatsEffectClass;

	// AI 행동 설정값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|AI")
	FAICombatSettings AICombatSettings;

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
	
	// 수풀(Bush) 진입/나감 처리
	// bInBush true면 수풀 속, false면 나옴
	UFUNCTION(BlueprintCallable, Category = "Brawl|Environment")
	void SetInBush(bool bInBush);

	// 현재 수풀 속에 있는지 여부 확인
	UFUNCTION(BlueprintCallable, Category = "Brawl|Environment")
	bool IsHiddenInBush() const { return bIsHiddenInBush; }

	// 수풀 속에서 누군가에 의해 감지되었는지 설정
	// bRevealed true면 감지됨(보임), false면 감지 안됨(숨음)
	void SetRevealed(bool bRevealed);

private:
	// 이동 속도 속성 변경 시 호출될 콜백
	void OnMovementSpeedChanged(const FOnAttributeChangeData& Data);

	// 실제 시각적 은신 상태 업데이트
	void UpdateMeshVisibility();

	// 수풀 속에 있는지 여부 (은신 가능 상태)
	bool bIsHiddenInBush = false;

	// 근처 적 등에 의해 위치가 발각되었는지 여부
	bool bIsRevealed = false;

	// 겹쳐진 수풀 개수 (여러 수풀이 겹쳐 있을 때 처리용)
	int32 BushOverlapCount = 0;
};

	
