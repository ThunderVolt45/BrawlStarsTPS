// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/BrawlProjectile_Spawner.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/EffectActors/BrawlAreaEffect.h"
#include "BrawlPoolSubsystem.h"

void ABrawlProjectile_Spawner::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 발사자 등 무시할 대상이면 처리하지 않음 (부모 클래스 로직 참조)
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator() 
		|| OtherActor == this || OtherActor->IsA(ABrawlProjectile::StaticClass())) return;

	// 명중 효과 재생 (Gameplay Cue)
	if (HitGameplayCueTag.IsValid())
	{
		AActor* SourceActor = GetInstigator();
		if (!SourceActor) SourceActor = GetOwner();
		
		if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor))
		{
			FGameplayCueParameters Params;
			Params.Location = Hit.Location;
			Params.Normal = Hit.Normal;
			Params.Instigator = SourceActor;
			
			// TargetActor가 null이면 ensure가 발생할 수 있으므로, 
			// EffectCauser를 통해 명시적으로 HitActor 혹은 SourceActor를 넘겨준다
			Params.EffectCauser = Hit.GetActor() ? Hit.GetActor() : SourceActor;

			SourceASC->ExecuteGameplayCue(HitGameplayCueTag, Params);
		}
	}
	
	// 서버에서만 스폰
	if (HasAuthority() && ActorClassToSpawn)
	{
		UWorld* World = GetWorld();
		UBrawlPoolSubsystem* PoolSubsystem = World ? World->GetSubsystem<UBrawlPoolSubsystem>() : nullptr;
		if (World)
		{
			// 스폰 위치 계산 (충돌 지점 + 법선 방향 오프셋)
			// ImpactPoint를 사용하여 발사체 크기에 상관없이 지면에 정확히 배치합니다.
			FVector SpawnLocation = Hit.ImpactPoint + (Hit.Normal * SpawnZOffset);
			FRotator SpawnRotation = FRotator::ZeroRotator;
			FTransform SpawnTransform(SpawnRotation, SpawnLocation);

			AActor* SpawnedActor = nullptr;
			if (PoolSubsystem)
			{
				SpawnedActor = PoolSubsystem->GetFromPool(ActorClassToSpawn, SpawnTransform, GetOwner(), GetInstigator());
			}
			else
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnedActor = World->SpawnActor<AActor>(ActorClassToSpawn, SpawnTransform, SpawnParams);
			}
			
			// 만약 스폰된 액터가 BrawlAreaEffect라면, 데미지 정보(SpecHandle) 전달
			if (ABrawlAreaEffect* AreaEffect = Cast<ABrawlAreaEffect>(SpawnedActor))
			{
				AreaEffect->EffectSpecHandle = this->DamageSpecHandle;
			}
		}
	}

	// 발사체 비활성화 (풀 반환)
	Deactivate();
}
