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
	
	// 발사체(WorldDynamic)끼리는 부딪히지 않고 겹치도록 설정
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	
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
	
	if (!DamageSpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Projectile Initialized with INVALID Damage Spec!"));
	}
}

void ABrawlProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 발사자(Instigator)는 무시
	if (AActor* MyInstigator = GetInstigator())
	{
		SphereComponent->IgnoreActorWhenMoving(MyInstigator, true);
	}

	// 발사체끼리(WorldDynamic)는 충돌하지 않고 통과하도록 강제 설정
	// (블루프린트 설정이 Block으로 되어 있어도 여기서 덮어씀)
	if (SphereComponent)
	{
		SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	}

	SetLifeSpan(1.5f);
}

void ABrawlProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()
		|| OtherActor == this || OtherActor->IsA(ABrawlProjectile::StaticClass()))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Projectile HIT Block: %s"), *OtherActor->GetName());
	ProcessHit(OtherActor, Hit.ImpactPoint);
	Destroy();
}

void ABrawlProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()
		|| OtherActor == this || OtherActor->IsA(ABrawlProjectile::StaticClass()))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Projectile HIT Overlap: %s"), *OtherActor->GetName());
	ProcessHit(OtherActor, GetActorLocation());
	Destroy();
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

