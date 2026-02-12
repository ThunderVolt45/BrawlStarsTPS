#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/BrawlSpawnPointType.h"
#include "BrawlSpawnPoint.generated.h"

/**
 * ABrawlSpawnPoint
 * 
 * 브롤러 또는 파워 큐브 상자가 생성될 위치를 지정하는 액터
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ABrawlSpawnPoint();

protected:
#if WITH_EDITORONLY_DATA
	// 에디터에서 시각적으로 구분하기 위한 빌보드 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Brawl|Spawn")
	TObjectPtr<class UBillboardComponent> BillboardComponent;
#endif

public:
	// 이 포인트에서 생성할 대상의 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Spawn")
	EBrawlSpawnPointType SpawnPointType = EBrawlSpawnPointType::Brawler;

	// (선택 사항) 브롤러인 경우 팀 ID 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Spawn", meta = (EditCondition = "SpawnPointType == EBrawlSpawnPointType::Brawler"))
	int32 TeamID = 0;

	/** 현재 이 위치가 다른 브롤러나 오브젝트에 의해 점유되어 있는지 확인 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|Spawn")
	bool IsOccupied(float CheckRadius = 100.0f) const;
};
