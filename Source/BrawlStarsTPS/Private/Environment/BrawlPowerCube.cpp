// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlPowerCube.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "BrawlCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "BrawlPoolSubsystem.h"

ABrawlPowerCube::ABrawlPowerCube()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	bIsActive = false; // 기본은 비활성 상태 (풀링 대기)

	// 1. CollisionSphere를 루트로 설정 (지형 충돌용, 작게)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(20.0f);
	
	// 물리 충돌 설정 (지면만 블록)
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// 2. PickupSphere (캐릭터 획득 감지용, 크게)
	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(CollisionSphere);
	PickupSphere->SetSphereRadius(60.0f);
	
	// [Fix] 다른 모든 채널을 확실하게 Ignore하고 오직 Pawn만 Overlap합니다.
	// 발사체가 주로 사용하는 WorldDynamic이나 Visibility 등과의 접점을 원천 차단합니다.
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);

	// 3. 메시 부착
	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
	CubeMesh->SetupAttachment(CollisionSphere);
	CubeMesh->SetCollisionProfileName(TEXT("NoCollision"));

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.0f, 90.0f, 0.0f);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
	ProjectileMovement->Friction = 0.5f;
	ProjectileMovement->ProjectileGravityScale = 1.5f;
	ProjectileMovement->bSweepCollision = true;

	// AI 감지 소스 컴포넌트 추가
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
	StimuliSourceComponent->bAutoRegister = true;
	StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

void ABrawlPowerCube::BeginPlay()
{
	Super::BeginPlay();
	
	// 서버에서만 충돌 이벤트를 처리하도록 권장
	if (HasAuthority() && PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ABrawlPowerCube::OnOverlapBegin);
	}

	// 처음부터 레벨에 배치된 경우를 위해 체크 (풀링 생성 시에는 bIsActive가 false이므로 여기서 안 불림)
	if (bIsActive)
	{
		OnActivate();
	}
}

void ABrawlPowerCube::OnActivate()
{
	bIsActive = true;
	SetActorHiddenInGame(false);
	
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (PickupSphere)
	{
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	if (RotatingMovement)
	{
		RotatingMovement->Activate();
	}

	// 톡 튀어오르는 효과 적용 (랜덤 방향)
	if (ProjectileMovement)
	{
		// 루트 컴포넌트를 기준으로 물리 이동 수행
		ProjectileMovement->SetUpdatedComponent(CollisionSphere);
		ProjectileMovement->StopMovementImmediately();
		
		FVector LaunchDir = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
		float LaunchStrength = FMath::RandRange(200.0f, 400.0f);
		float UpStrength = FMath::RandRange(400.0f, 600.0f);

		FVector LaunchVelocity = (LaunchDir * LaunchStrength) + (FVector::UpVector * UpStrength);
		
		// 2. 속도 부여 및 시뮬레이션 시작
		ProjectileMovement->Velocity = LaunchVelocity;
		ProjectileMovement->Activate(true); // true: 강제 리셋 활성화
	}

	if (SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
	}
}

void ABrawlPowerCube::OnDeactivate()
{
	bIsActive = false;
	SetActorHiddenInGame(true);

	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (PickupSphere)
	{
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (RotatingMovement)
	{
		RotatingMovement->Deactivate();
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
}

void ABrawlPowerCube::Deactivate()
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

void ABrawlPowerCube::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버가 아니면 리턴
	if (!HasAuthority()) return;

	if (!OtherActor || OtherActor == this) return;

	// 브롤 캐릭터 확인
	ABrawlCharacter* Character = Cast<ABrawlCharacter>(OtherActor);
	if (!Character || Character->IsDead()) return;
	
	UE_LOG(LogTemp, Log, TEXT("PowerCube Overlapped with: %s"), *OtherActor->GetName());

	// GAS 컴포넌트 가져오기
	UAbilitySystemComponent* TargetASC = Character->GetAbilitySystemComponent();
	if (TargetASC)
	{
		if (PowerCubeEffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(PowerCubeEffectClass, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				
				if (PickupSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
				}

				if (PickupVFX)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupVFX, GetActorLocation());
				}

				// 획득 성공 후 제거 대신 비활성화 (풀 반환)
				Deactivate();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABrawlPowerCube: PowerCubeEffectClass is NOT SET! Set it in Blueprint."));
		}
	}
}
