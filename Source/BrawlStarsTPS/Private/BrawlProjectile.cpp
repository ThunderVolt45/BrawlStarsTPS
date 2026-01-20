// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

ABrawlProjectile::ABrawlProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 1. 충돌체 설정
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	
	// 테스트를 위해 모든 채널 Block 설정
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);
	
	// 겹침 이벤트도 일단 켜둠
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
}

void ABrawlProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (AActor* MyInstigator = GetInstigator())
	{
		SphereComponent->IgnoreActorWhenMoving(MyInstigator, true);
	}

	SetLifeSpan(2.0f);
}

void ABrawlProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == this) return;

	UE_LOG(LogTemp, Warning, TEXT("Projectile HIT Block: %s"), *OtherActor->GetName());
	ProcessHit(OtherActor, Hit.ImpactPoint);
	Destroy();
}

void ABrawlProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == this) return;

	UE_LOG(LogTemp, Warning, TEXT("Projectile HIT Overlap: %s"), *OtherActor->GetName());
	ProcessHit(OtherActor, GetActorLocation());
	Destroy();
}

void ABrawlProjectile::ProcessHit(AActor* OtherActor, const FVector& HitLocation)
{
	// 디버그 구체 그리기 (충돌 확인용)
	if (GetWorld())
	{
		DrawDebugSphere(GetWorld(), HitLocation, 10.0f, 12, FColor::Red, false, 2.0f);
	}

	// 데미지 적용
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
	{
		if (DamageSpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
		}
	}
}

