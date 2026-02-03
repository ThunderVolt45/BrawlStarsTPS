// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlPowerCube.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "BrawlCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

ABrawlPowerCube::ABrawlPowerCube()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 서버-클라이언트 동기화 활성화

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
	CubeMesh->SetupAttachment(SceneRoot);
	CubeMesh->SetCollisionProfileName(TEXT("NoCollision"));

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(SceneRoot);
	PickupSphere->SetSphereRadius(60.0f);
	
	// 명시적 충돌 설정
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.0f, 90.0f, 0.0f);

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

	if (SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
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

				// 획득 성공 후 제거
				Destroy();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABrawlPowerCube: PowerCubeEffectClass is NOT SET! Set it in Blueprint."));
		}
	}
}
