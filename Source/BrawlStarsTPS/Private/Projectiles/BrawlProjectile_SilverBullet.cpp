#include "Projectiles/BrawlProjectile_SilverBullet.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABrawlProjectile_SilverBullet::ABrawlProjectile_SilverBullet()
{
	// 1. 관통 구현을 위해 모든 채널에 대해 Overlap으로 설정
	// (Block되면 물리적으로 멈추므로 관통 효과를 내기 어려움)
	if (SphereComponent)
	{
		SphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	}

	// 2. 속도 설정 (일반 탄환과 동일하거나 더 빠르게 설정 가능)
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 3500.0f; // 약간 더 빠르게
		ProjectileMovement->MaxSpeed = 3500.0f;
	}
}

void ABrawlProjectile_SilverBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Block이 발생했을 때 호출됨 (혹시 설정이 꼬여서 Block되어도 관통 처리)
	// Super::OnHit()은 호출하지 않음 (Destroy 방지)

	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

	if (!HitActors.Contains(OtherActor))
	{
		ProcessHit(OtherActor, Hit.ImpactPoint);
		HitActors.Add(OtherActor);
		
		UE_LOG(LogTemp, Log, TEXT("SilverBullet Hit (Block): %s"), *OtherActor->GetName());
	}
}

void ABrawlProjectile_SilverBullet::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 관통의 핵심 로직
	// Super::OnBeginOverlap()은 호출하지 않음 (Destroy 방지)

	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

	if (!HitActors.Contains(OtherActor))
	{
		// ProcessHit 내부에서 데미지 적용
		ProcessHit(OtherActor, GetActorLocation());
		HitActors.Add(OtherActor);

		UE_LOG(LogTemp, Log, TEXT("SilverBullet Pierced: %s"), *OtherActor->GetName());

		// TODO: 만약 '파괴 가능한 벽'이라면 파괴 로직 추가
		// if (OtherActor->Implements<UBreakableInterface>()) ...
	}
}
