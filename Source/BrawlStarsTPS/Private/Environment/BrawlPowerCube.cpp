// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlPowerCube.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "BrawlCharacter.h"
#include "Kismet/GameplayStatics.h"

ABrawlPowerCube::ABrawlPowerCube()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
	CubeMesh->SetupAttachment(SceneRoot);
	CubeMesh->SetCollisionProfileName(TEXT("NoCollision")); // 메시는 충돌 없음

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(SceneRoot);
	PickupSphere->SetSphereRadius(60.0f);
	PickupSphere->SetCollisionProfileName(TEXT("Trigger")); // 폰만 감지하도록 설정 필요 (OverlapAllDynamic 등)

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.0f, 90.0f, 0.0f);
}

void ABrawlPowerCube::BeginPlay()
{
	Super::BeginPlay();
	
	if (PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ABrawlPowerCube::OnOverlapBegin);
	}

	if (SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
	}

	// 애니메이션: 살짝 위아래로 둥둥 떠다니는 효과 추가 가능 (Timeline이나 Tick에서)
	// 일단 RotatingMovement로 회전만 적용됨.
}

void ABrawlPowerCube::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	// 브롤 캐릭터만 획득 가능
	ABrawlCharacter* Character = Cast<ABrawlCharacter>(OtherActor);
	if (!Character) return;
	
	// 사망한 캐릭터는 획득 불가
	if (Character->IsDead()) return;

	// GAS 컴포넌트 가져오기
	UAbilitySystemComponent* TargetASC = Character->GetAbilitySystemComponent();
	if (TargetASC && PowerCubeEffectClass)
	{
		FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(PowerCubeEffectClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			
			// 효과음 재생
			if (PickupSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
			}

			// VFX 재생
			if (PickupVFX)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupVFX, GetActorLocation());
			}

			// 획득 후 파괴
			Destroy();
		}
	}
}
