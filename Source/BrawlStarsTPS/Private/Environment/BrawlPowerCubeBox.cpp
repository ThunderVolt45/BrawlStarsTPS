// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlPowerCubeBox.h"
#include "BrawlAbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Environment/BrawlPowerCube.h"
#include "Kismet/GameplayStatics.h"
#include "UI/BrawlHealthWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "BrawlPoolSubsystem.h"

ABrawlPowerCubeBox::ABrawlPowerCubeBox()
{
	// 체력바 빌보드(카메라 방향 회전)를 위해 Tick 활성화
	PrimaryActorTick.bCanEverTick = true;

	// 캐릭터 설정 조정
	GetCapsuleComponent()->SetCapsuleSize(50.f, 50.f);
	GetCapsuleComponent()->SetCollisionProfileName(FName("BlockAll"));

	// 상자용 StaticMeshComponent 생성
	BoxMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMeshComponent"));
	BoxMeshComponent->SetupAttachment(GetCapsuleComponent());
	BoxMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 불필요한 스켈레탈 메시 숨기기
	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true);
	}

	// 체력바 설정 수정
	if (HealthBarComponent)
	{
		// 절대 좌표 사용을 명시적으로 끔
		HealthBarComponent->SetUsingAbsoluteLocation(false);
		HealthBarComponent->SetUsingAbsoluteRotation(false);
		HealthBarComponent->SetUsingAbsoluteScale(false);

		// 부착 지점을 캡슐로 변경
		HealthBarComponent->SetupAttachment(GetCapsuleComponent());
		
		// 상대 좌표로 위치 설정
		HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
		
		HealthBarComponent->SetOwnerNoSee(false); 
		HealthBarComponent->SetHiddenInGame(false);
	}

	// 불필요한 컴포넌트 비활성화
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

	// AI 감지 소스 (시각)
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
	StimuliSourceComponent->bAutoRegister = true;
	StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());

	// 팀 ID 설정 (기본 중립)
	TeamID = 255;
	CharacterID = FName("PowerCubeBox");
	CharacterType = EBrawlCharacterType::Etc;
}

void ABrawlPowerCubeBox::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// 상자는 컨트롤러에 의해 Possess되지 않으므로 여기서 직접 GAS 및 위젯 초기화
	InitAbilityActorInfo();
}

void ABrawlPowerCubeBox::BeginPlay()
{
	Super::BeginPlay();

	// 생성자에서 설정한 상대 위치가 덮어씌워지지 않도록 다시 한번 확인
	if (HealthBarComponent)
	{
		HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
		HealthBarComponent->SetHiddenInGame(false);
	}

	// 수동으로 속성 초기화
	if (AbilitySystemComponent && AttributeSet)
	{
		if (HasAuthority())
		{
			AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetMaxHealthAttribute(), DefaultMaxHealth);
			AbilitySystemComponent->SetNumericAttributeBase(UBrawlAttributeSet::GetHealthAttribute(), DefaultMaxHealth);
		}
	}
}

void ABrawlPowerCubeBox::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// ABrawlCharacter의 OnHealthChanged는 사망 처리를 담당함
	Super::OnHealthChanged(Data);
}

void ABrawlPowerCubeBox::Die()
{
	if (bIsDeadInternal) return;
	bIsDeadInternal = true;

	UBrawlPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UBrawlPoolSubsystem>();

	// 파워 큐브 드롭
	if (PowerCubeClass)
	{
		if (PoolSubsystem)
		{
			PoolSubsystem->GetFromPool(PowerCubeClass, GetActorTransform());
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			GetWorld()->SpawnActor<ABrawlPowerCube>(PowerCubeClass, GetActorTransform(), SpawnParams);
		}
	}

	// 파괴 연출 (ABrawlObstacle 로직 이식)
	if (DestructionEffectClass)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, -50.0f);
		FRotator SpawnRotation = GetActorRotation();
		FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		
		AActor* SpawnedActor = nullptr;
		if (PoolSubsystem)
		{
			SpawnedActor = PoolSubsystem->GetFromPool(DestructionEffectClass, SpawnTransform);
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnedActor = GetWorld()->SpawnActor<AActor>(DestructionEffectClass, SpawnLocation, SpawnRotation, SpawnParams);
		}

		if (SpawnedActor)
		{
			// 지오메트리 컬렉션 컴포넌트를 찾아 물리적 충격 가하기 (즉시 파괴 연출)
			if (UGeometryCollectionComponent* GCComp = SpawnedActor->FindComponentByClass<UGeometryCollectionComponent>())
			{
				GCComp->AddRadialImpulse(GetActorLocation(), 180, 1000, ERadialImpulseFalloff::RIF_Linear, true);
			}
		}
	}

	if (DestructionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestructionSFX, GetActorLocation());
	}

	// 캐릭터 사망 처리
	Destroy();
}

void ABrawlPowerCubeBox::GetPrewarmRequirements(TMap<TSubclassOf<AActor>, int32>& OutRequirements, int32 BaseCount) const
{
	if (PowerCubeClass)
	{
		OutRequirements.FindOrAdd(PowerCubeClass) += BaseCount;
	}
	if (DestructionEffectClass)
	{
		OutRequirements.FindOrAdd(DestructionEffectClass) += BaseCount;
	}
}
