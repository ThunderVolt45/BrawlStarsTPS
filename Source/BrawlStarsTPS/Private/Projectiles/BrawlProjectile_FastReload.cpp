#include "Projectiles/BrawlProjectile_FastReload.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlCharacter.h"

#include "Components/SphereComponent.h"

void ABrawlProjectile_FastReload::ProcessHit(AActor* OtherActor, const FVector& HitLocation)
{
	// 1. 자기 자신이나 발사자(Instigator)에게 맞았을 경우 무시
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator() || OtherActor == this)
	{
		return;
	}

	// 2. 기본 데미지 처리 (적에게 맞으면 데미지도 줌)
	Super::ProcessHit(OtherActor, HitLocation);
	
	// 명중 대상이 BrawlCharacter가 아닐 경우 여기서 중단
	if (!OtherActor->IsA(ABrawlCharacter::StaticClass()))
		return;
	
	// 3. 탄환 회복
	AActor* MyInstigator = GetInstigator();
	UAbilitySystemComponent* ASC = MyInstigator->FindComponentByClass<UAbilitySystemComponent>();
	
	if (!MyInstigator || !ASC) return;
	
	bool bFoundAmmo = false;
	bool bFoundMaxAmmo = false;
	float CurrentAmmo = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetAmmoAttribute(), bFoundAmmo);
	float MaxAmmo = ASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxAmmoAttribute(), bFoundMaxAmmo);

	if (bFoundAmmo && bFoundMaxAmmo)
	{
		// 현재 탄환이 최대치보다 적을 때만 회복
		if (CurrentAmmo < MaxAmmo)
		{
			// 회복 후 최대치를 넘지 않도록 조정
			float NewAmmo = FMath::Min(CurrentAmmo + ReloadAmount, MaxAmmo);
			float Delta = NewAmmo - CurrentAmmo;

			if (Delta > 0.0f)
			{
				ASC->ApplyModToAttributeUnsafe(UBrawlAttributeSet::GetAmmoAttribute(), EGameplayModOp::Additive, Delta);
				UE_LOG(LogTemp, Log, TEXT("FastReload Hit! Recovered %.1f Ammo (Total: %.1f)"), Delta, NewAmmo);
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("FastReload Hit! But Ammo is Full."));
		}
	}
}