// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

ABrawlProjectile::ABrawlProjectile()
{
	PrimaryActorTick.bCanEverTick = true; // 관통 및 레이캐스트 로직을 위해 Tick 활성화
	bReplicates = true;

	// 1. 충돌체 설정
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);
	
	// 발사체끼리(WorldDynamic)는 부딪히지 않고 겹치도록 설정
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	
	// 겹침 이벤트도 켜둠
	SphereComponent->SetGenerateOverlapEvents(true);
	
	// 이벤트 바인딩
	SphereComponent->OnComponentHit.AddDynamic(this, &ABrawlProjectile::OnHit);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABrawlProjectile::OnBeginOverlap);

	// 2. 외형 설정
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(SphereComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3. 이동 컴포넌트 설정
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ABrawlProjectile::InitializeProjectile(const FGameplayEffectSpecHandle& InDamageSpecHandle)
{
	DamageSpecHandle = InDamageSpecHandle;
	
	if (!DamageSpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Projectile Initialized with INVALID Damage Spec!"));
	}
}

void ABrawlProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	PreviousLocation = GetActorLocation();

	// 관통형 발사체는 직접 충돌을 검사해야하므로 Tick 활성화
	SetActorTickEnabled(bCanPierce);
	
	// 발사자(Instigator)는 무시
	if (AActor* MyInstigator = GetInstigator())
	{
		SphereComponent->IgnoreActorWhenMoving(MyInstigator, true);
	}

	if (SphereComponent)
	{
		// 충돌 활성화 강제 (QueryOnly: 물리 시뮬레이션 없이 오버랩/히트 감지)
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
		
		// 블루프린트 설정 무시하고 강제로 Overlap 이벤트 활성화
		SphereComponent->SetGenerateOverlapEvents(true);

		if (bCanPierce)
		{
			// 관통형 발사체는 물리적으로 멈추면 안 되므로 모든 채널과 Overlap 해야 함
			SphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
		}
		else
		{
			// 일반 발사체는 발사체(WorldDynamic)끼리만 충돌 무시(Overlap)
			SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		}
	}

	SetLifeSpan(LifeTime);
}

void ABrawlProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 관통형 발사체가 아니라면 중단
	if (!bCanPierce || !SphereComponent)
	{
		return;
	}

	// 관통형 발사체인 경우, 이동 경로에 대한 스윕(Sweep) 검사 수행 (터널링 방지)
	FVector CurrentLocation = GetActorLocation();

	// 움직임이 거의 없으면 스킵
	if (FVector::DistSquared(PreviousLocation, CurrentLocation) < 1.0f)
	{
		return;
	}

	// 레이캐스트 준비
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	
	// 발사자(Instigator)는 무시
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
	if (GetInstigator()) QueryParams.AddIgnoredActor(GetInstigator());

	// Pawn과 WorldStatic, WorldDynamic에 대해 검사
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	// 구체 스윕 (경로상 모든 물체 감지)
	bool bHit = GetWorld()->SweepMultiByObjectType(
		HitResults,
		PreviousLocation,
		CurrentLocation,
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(SphereComponent->GetScaledSphereRadius()),
		QueryParams
	);

	if (bHit)
	{
		for (const FHitResult& Result : HitResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && !HitActors.Contains(HitActor))
			{
				// 발사체끼리는 무시
				if (HitActor->IsA(ABrawlProjectile::StaticClass())) continue;

				UE_LOG(LogTemp, Warning, TEXT("Projectile Sweep Hit: %s"), *HitActor->GetName());

				// 처리 등록
				HitActors.Add(HitActor);
				ProcessHit(HitActor, Result.ImpactPoint);

				if (bDestroyObstacles)
				{
					// TODO: 파괴 로직
				}
			}
		}
	}

	PreviousLocation = CurrentLocation;
}


void ABrawlProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 발사자(Instigator)는 무시
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator() 
		|| OtherActor == this || OtherActor->IsA(ABrawlProjectile::StaticClass())) return;

	// 이미 처리된 액터면 무시 (관통 시 중복 방지)
	if (HitActors.Contains(OtherActor)) return;
	HitActors.Add(OtherActor);

	UE_LOG(LogTemp, Warning, TEXT("Projectile HIT Block: %s"), *OtherActor->GetName());
	ProcessHit(OtherActor, Hit.ImpactPoint);

	if (bDestroyObstacles)
	{
		// TODO: 장애물 파괴 로직
	}

	if (!bCanPierce)
	{
		Destroy();
	}
}

void ABrawlProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 발사자(Instigator)는 무시
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()
		|| OtherActor == this || OtherActor->IsA(ABrawlProjectile::StaticClass()))
	{
		return;
	}

	// 이미 처리된 액터면 무시
	if (HitActors.Contains(OtherActor)) return;
	HitActors.Add(OtherActor);

	UE_LOG(LogTemp, Warning, TEXT("Projectile HIT Overlap: %s"), *OtherActor->GetName());
	ProcessHit(OtherActor, GetActorLocation());

	if (bDestroyObstacles)
	{
		// TODO: 장애물 파괴 로직
	}

	if (!bCanPierce)
	{
		Destroy();
	}
}

void ABrawlProjectile::ProcessHit(AActor* OtherActor, const FVector& HitLocation)
{
	if (!OtherActor) return;
	
	// 디버그 구체 그리기
	if (GetWorld())
	{
		DrawDebugSphere(GetWorld(), HitLocation, 10.0f, 12, FColor::Red, false, 2.0f);
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	
	if (!TargetASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessHit: Target [%s] has NO AbilitySystemComponent!"), *OtherActor->GetName());
		return;
	}

	if (DamageSpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGE = TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ProcessHit: DamageSpecHandle is INVALID! Cannot apply damage."));
	}
}