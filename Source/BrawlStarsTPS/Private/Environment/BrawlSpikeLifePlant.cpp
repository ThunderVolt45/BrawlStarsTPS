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

ABrawlSpikeLifePlant::ABrawlSpikeLifePlant()
{
	PrimaryActorTick.bCanEverTick = true;

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

	// 1. 소환사의 팀 ID 상속
	if (ABrawlCharacter* Summoner = Cast<ABrawlCharacter>(GetInstigator()))
	{
		SetGenericTeamId(FGenericTeamId(Summoner->GetTeamID()));
		UE_LOG(LogTemp, Log, TEXT("SpikeLifePlant: Inherited TeamID %d from Summoner %s"), GetTeamID(), *Summoner->GetName());
	}

	// 2. 체력 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxHealthAttribute(), DefaultMaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHealthAttribute(), DefaultMaxHealth);
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
	if (bIsDeadInternal) return;
	bIsDeadInternal = true;

	// 주변 아군 치유
	HealNearbyAllies();

	// 파괴 처리
	Destroy();
}
