#pragma once

#include "CoreMinimal.h"
#include "BrawlProjectile.h"
#include "BrawlProjectile_FastReload.generated.h"

/**
 * 쾌속 장전기 (Speedloader) 가젯용 발사체
 * - 적중 시 발사자의 탄환을 2발 회복합니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlProjectile_FastReload : public ABrawlProjectile
{
	GENERATED_BODY()
	
public:
	// 적중 처리 로직 오버라이드
	virtual void ProcessHit(AActor* OtherActor, const FVector& HitLocation) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|Gadget")
	float ReloadAmount = 1.0f;
};
