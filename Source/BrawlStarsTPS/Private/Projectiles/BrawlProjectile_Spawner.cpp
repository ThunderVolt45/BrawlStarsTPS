// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/BrawlProjectile_Spawner.h"
#include "Abilities/EffectActors/BrawlAreaEffect.h"

void ABrawlProjectile_Spawner::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 발사자 등 무시할 대상이면 처리하지 않음 (부모 클래스 로직 참조)
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator() 
		|| OtherActor == this || OtherActor->IsA(ABrawlProjectile::StaticClass())) return;

	// 서버에서만 스폰
	if (HasAuthority() && ActorClassToSpawn)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// 스폰 위치 계산 (충돌 지점 + 법선 방향 오프셋)
			// 보통 장판은 바닥(Hit.Normal이 위쪽)에 깔리므로 Location + ZOffset
			FVector SpawnLocation = Hit.Location + (Hit.Normal * SpawnZOffset);
			FRotator SpawnRotation = FRotator::ZeroRotator; // 보통 장판은 회전 없이(0,0,0) 생성

			AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
			
			// 만약 스폰된 액터가 BrawlAreaEffect라면, 데미지 정보(SpecHandle) 전달
			if (ABrawlAreaEffect* AreaEffect = Cast<ABrawlAreaEffect>(SpawnedActor))
			{
				AreaEffect->EffectSpecHandle = this->DamageSpecHandle;
			}
		}
	}

	// 발사체 파괴
	Destroy();
}
