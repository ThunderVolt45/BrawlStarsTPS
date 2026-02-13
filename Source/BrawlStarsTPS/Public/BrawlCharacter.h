// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "Data/BrawlCharacterData.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxCombatRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PreferredCombatRange = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MinCombatRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float FleeHealthRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ResumeCombatHealthRatio = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PursuitTargetHealthRatio = 0.2f;
};

/**
 * ABrawlCharacter
 *
 * 브롤스타즈 TPS 프로젝트의 기본 캐릭터 클래스
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlCharacter : public ACharacter, 
	public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// --- Core ---
	ABrawlCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Interface Implementations ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	// --- Brawler Stats & Data ---
	UFUNCTION(BlueprintCallable, Category = "Brawl|Character")
	FName GetCharacterID() const { return CharacterID; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Character")
	FBrawlCharacterData GetCharacterData() const;

	UFUNCTION(BlueprintCallable, Category = "Brawl|Character")
	UTexture2D* GetCharacterIcon() const;

	UFUNCTION(BlueprintCallable, Category = "Brawl|Character")
	int32 GetTeamID() const { return TeamID; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Character")
	bool IsAlly(AActor* Other) const;

	// --- Combat & Abilities ---
	UFUNCTION(BlueprintCallable, Category = "Brawl|Combat")
	void NotifyCombatAction();

	UFUNCTION(BlueprintCallable, Category = "Brawl|Combat")
	void NotifyHyperChargeActivated();

	UFUNCTION(BlueprintCallable, Category = "Brawl|Combat")
	float GetEstimatedProjectileSpeed() const { return EstimatedProjectileSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Combat")
	float GetEstimatedProjectileLifetime() const { return EstimatedProjectileLifetime; }

	// --- Health & Life Cycle ---
	UFUNCTION(BlueprintCallable, Category = "Brawl|Health")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "Brawl|Health")
	bool IsDead() const { return bIsDead; }

	void SetBrawlerActive(bool bActive);
	void RespawnAt(FVector Location, FRotator Rotation);
	void SetLastHitInstigator(AActor* InInstigator) { LastHitInstigator = InInstigator; }

	// --- Environment & Perception ---
	UFUNCTION(BlueprintCallable, Category = "Brawl|Environment")
	void SetInBush(bool bInBush);

	UFUNCTION(BlueprintCallable, Category = "Brawl|Environment")
	bool IsHiddenInBush() const { return bIsHiddenInBush; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|Environment")
	virtual bool IsVisibleTo(const FGenericTeamId& ObserverTeam) const;

	void SetRevealed(bool bRevealed);

	// --- AI Support ---
	UFUNCTION(BlueprintCallable, Category = "Brawl|AI")
	const FAICombatSettings& GetAICombatSettings() const { return AICombatSettings; }

	UFUNCTION(BlueprintCallable, Category = "Brawl|AI")
	class UBehaviorTree* GetCombatBehaviorTree() const;

	// --- Components & Getters ---
	UBrawlAbilitySystemComponent* GetBrawlAbilitySystemComponent() const { return AbilitySystemComponent; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() { return FollowCamera; }

protected:
	// --- Internal Setup ---
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitAbilityActorInfo();
	void InitializeAttributes();

	// --- Callbacks & Replication ---
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);
	virtual void OnCombatRevealedTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void OnHyperChargeTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	UFUNCTION()
	void OnMatchStateChanged();

	UFUNCTION()
	void OnRep_IsDead();

	// --- Visual Effects ---
	void UpdatePoisonScreenEffect(float DeltaTime);
	void UpdateMeshVisibility();
	void ApplyCombatRevealEffect();

protected:
	// --- Data Assets ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	FName CharacterID = FName("Colt");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Stats")
	TObjectPtr<UDataTable> CharacterDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Stats")
	TObjectPtr<UDataTable> AIDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|AI")
	FAICombatSettings AICombatSettings;

	// --- GAS Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Character")
	TObjectPtr<UBrawlAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UBrawlAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Character")
	TObjectPtr<UBrawlHeroComponent> HeroComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brawl|Abilities")
	TArray<TSubclassOf<UBrawlGameplayAbility>> StartupAbilities;

	// --- Gameplay Tags & Effects ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Combat")
	FGameplayTag CombatTag = FGameplayTag::RequestGameplayTag(FName("State.Combat"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Combat")
	FGameplayTag RevealedTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Revealed"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Combat")
	FGameplayTag HyperChargeTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|GameplayCue")
	FGameplayTag RespawnCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Common.Respawn"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|GameplayCue")
	FGameplayTag DeathCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Common.Die"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Combat")
	TSubclassOf<UGameplayEffect> CombatRevealEffectClass;

	// 사망 시 취소할 어빌리티 태그 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Death")
	FGameplayTagContainer CancelAbilitiesWithTags;
	
	// --- Visuals & Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|UI")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brawl|Effects")
	TObjectPtr<class UPostProcessComponent> PoisonPostProcess;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Effects")
	TObjectPtr<UMaterialInterface> PoisonPPMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Effects")
	TSubclassOf<AActor> HyperChargeEffectClass;

	// --- Combat Tuning ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Combat")
	float EstimatedProjectileSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Combat")
	float EstimatedProjectileLifetime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Camera")
	float ControlledMeshYawOffset = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Brawl|Debug")
	bool DrawDebugMuzzleLine = false;

protected:
	// --- Internal Variables (Accessible to derived classes) ---
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Brawl|Stats")
	uint8 TeamID = 255;

private:
	void OnMovementSpeedChanged(const FOnAttributeChangeData& Data);

private:
	// --- Internal Variables ---
	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleInstanceOnly, Category = "Brawl|Health")
	bool bIsDead = false;

	UPROPERTY()
	TObjectPtr<AActor> LastHitInstigator;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> PoisonPPMaterialInstance;

	UPROPERTY()
	TObjectPtr<AActor> HyperChargeEffectInstance;

	UPROPERTY()
	TObjectPtr<class ABrawlPoisonZone> CachedPoisonZone;

	float CurrentPoisonIntensity = 0.0f;
	bool bIsHiddenInBush = false;
	bool bIsRevealed = false;
	bool bIsRevealedByCombat = false;
	bool bIsCombatState = false;
	int32 BushOverlapCount = 0;

	// Delegates
	FDelegateHandle CombatRevealedTagDelegateHandle;
	FDelegateHandle CombatStateTagDelegateHandle;
	FDelegateHandle HyperChargeTagDelegateHandle;
};
