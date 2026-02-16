// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlSpikeLifePlant.h"
#include "BrawlAbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Camera/CameraComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/SpringArmComponent.h"
#include "BrawlPoolSubsystem.h"
#include "BrawlStarsTPSGameMode.h"

ABrawlSpikeLifePlant::ABrawlSpikeLifePlant()
{
	PrimaryActorTick.bCanEverTick = true;

	bIsActive = true;

	// 1. 충돌체 설정
	GetCapsuleComponent()->SetCapsuleSize(45.f, 60.f);
	GetCapsuleComponent()->SetCollisionProfileName(FName("BlockAll"));

	// 2. 외형 메시 (StaticMesh 사용)
	PlantMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlantMeshComponent"));
	PlantMeshComponent->SetupAttachment(GetCapsuleComponent());
	PlantMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3. 기본 캐릭터 요소 비활성화
	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->GravityScale = 0.0f;
	}

	if (GetCameraBoom())
	{
		GetCameraBoom()->DestroyComponent();
	}
	if (GetFollowCamera())
	{
		GetFollowCamera()->DestroyComponent();
	}

	// 4. 체력바 설정
	if (HealthBarComponent)
	{
		HealthBarComponent->SetupAttachment(GetCapsuleComponent());
		HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
		HealthBarComponent->SetOwnerNoSee(false);
		HealthBarComponent->SetHiddenInGame(false);
	}

	// 5. AI 감지 설정
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
	StimuliSourceComponent->bAutoRegister = true;
	StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());

	// 팀 ID 및 캐릭터 ID 설정
	TeamID = 255; // 기본 중립 (소환 시점에 소환사의 팀으로 변경 필요)
	CharacterID = FName("SpikeLifePlant");
	CharacterType = EBrawlCharacterType::Summon;
}

void ABrawlSpikeLifePlant::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// 소환물은 컨트롤러가 없으므로 직접 초기화
	InitAbilityActorInfo();
}

void ABrawlSpikeLifePlant::BeginPlay()
{
	Super::BeginPlay();

	if (bIsActive)
	{
		OnActivate();
	}
}

void ABrawlSpikeLifePlant::OnActivate()
{
	bIsActive = true;
	bIsDead = false;
	
	SetBrawlerActive(true);

	// 1. 소환사의 팀 ID 상속
	if (ABrawlCharacter* Summoner = Cast<ABrawlCharacter>(GetInstigator()))
	{
		SetGenericTeamId(FGenericTeamId(Summoner->GetTeamID()));
		// UE_LOG(LogTemp, Log, TEXT("SpikeLifePlant: Inherited TeamID %d from Summoner %s"), GetTeamID(), *Summoner->GetName());
	}

	// 2. 체력 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxHealthAttribute(), DefaultMaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHealthAttribute(), DefaultMaxHealth);
	}

	// 3. 가시성 강제 업데이트
	UpdateMeshVisibility();
}

void ABrawlSpikeLifePlant::OnDeactivate()
{
	bIsActive = false;
	bIsDead = true;
	
	SetBrawlerActive(false);
}

void ABrawlSpikeLifePlant::Deactivate()
{
	if (UBrawlPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UBrawlPoolSubsystem>())
	{
		PoolSubsystem->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}

void ABrawlSpikeLifePlant::HealNearbyAllies()
{
	if (!HasAuthority() || !AbilitySystemComponent || !HealEffectGEClass) return;

	FVector Origin = GetActorLocation();
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(HealRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoredActors);

	// 치유용 GE Spec 생성
	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(HealEffectGEClass, 1.0f, ContextHandle);

	if (SpecHandle.IsValid())
	{
		// SetByCaller를 통해 치유량 전달 (태그는 프로젝트 컨벤션인 Data.Heal 사용)
		static FGameplayTag HealTag = FGameplayTag::RequestGameplayTag(FName("Data.Heal"));
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(HealTag, HealAmount);

		// 주변의 모든 캐릭터 탐색
		if (GetWorld()->OverlapMultiByChannel(OverlapResults, Origin, FQuat::Identity, ECC_Pawn, Sphere, QueryParams))
		{
			for (const FOverlapResult& Result : OverlapResults)
			{
				if (ABrawlCharacter* TargetChar = Cast<ABrawlCharacter>(Result.GetActor()))
				{
					if (TargetChar->IsDead()) continue;

					bool bShouldHeal = false;
					
					// 1. 팀이 설정된 경우 (0 또는 1): 같은 팀원 치유
					if (GetTeamID() != 255)
					{
						bShouldHeal = (TargetChar->GetTeamID() == GetTeamID());
					}
					// 2. 팀이 없는 경우 (255, 예: 쇼다운 중립): 자신을 생성한 브롤러(Instigator)만 치유
					else
					{
						bShouldHeal = (TargetChar == GetInstigator());
					}

					if (bShouldHeal)
					{
						if (UAbilitySystemComponent* TargetASC = TargetChar->GetAbilitySystemComponent())
						{
							TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
						}
					}
				}
			}
		}
	}

	// 시각적 피드백 (GameplayCue 실행)
	if (AbilitySystemComponent && HealCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		CueParams.Instigator = GetInstigator();
		
		AbilitySystemComponent->ExecuteGameplayCue(HealCueTag, CueParams);
	}
}
void ABrawlSpikeLifePlant::Die()
{
	if (bIsDead) return;
	
	// Super::Die()를 호출하면 bIsDead = true가 되고 GameMode에 알림이 감
	// 하지만 Summon은 Respawn하지 않으므로 Super::Die()에서 Destroy될 수 있음.
	// 따라서 여기서 직접 처리하거나 Super::Die()의 로직을 제어해야 함.
	
	bIsDead = true;

	// 사망 GameplayCue 호출
	if (AbilitySystemComponent && DeathCueTag.IsValid())
	{
		AbilitySystemComponent->ExecuteGameplayCue(DeathCueTag);
	}

	// 주변 아군 치유
	HealNearbyAllies();

	// 서버에서 알림 (Summon은 킬 카운트 등에 반영될 수 있음)
	if (HasAuthority())
	{
		if (ABrawlStarsTPSGameMode* GM = GetWorld()->GetAuthGameMode<ABrawlStarsTPSGameMode>())
		{
			GM->NotifyKill(LastHitInstigator, this);
		}
	}

	// 파괴 처리 대신 비활성화 (풀 반환)
	Deactivate();
}
