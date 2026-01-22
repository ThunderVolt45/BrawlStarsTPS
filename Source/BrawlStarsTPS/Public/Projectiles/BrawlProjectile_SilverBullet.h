#pragma once

#include "CoreMinimal.h"
#include "BrawlProjectile.h"
#include "BrawlProjectile_SilverBullet.generated.h"

/**
 * 실버 불릿 (Silver Bullet) 가젯용 발사체
 * - 벽과 적을 관통합니다.
 * - 단일 대상에게 강력한 피해를 주며 사라지지 않습니다.
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlProjectile_SilverBullet : public ABrawlProjectile
{
	GENERATED_BODY()
	
public:
	ABrawlProjectile_SilverBullet();

protected:
	// 관통을 위해 부모의 파괴 로직(Destroy)을 차단하고 오버라이드
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	// 이미 피격된 액터 목록 (다단히트 방지용)
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HitActors;
};
