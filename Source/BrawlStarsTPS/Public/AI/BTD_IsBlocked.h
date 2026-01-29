// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IsBlocked.generated.h"

/**
 * UBTD_IsBlocked
 * 
 * 캐릭터가 이동하려는 방향에 장애물이 있는지 검사하는 데코레이터입니다.
 * - 이동 중일 때: 현재 속도(Velocity) 방향 검사
 * - 정지 상태일 때: 캐릭터 전방(Forward) 방향 검사
 */
UCLASS()
class BRAWLSTARSTPS_API UBTD_IsBlocked : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_IsBlocked();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	// 검사할 거리 (cm)
	UPROPERTY(EditAnywhere, Category = "Condition")
	float CheckDistance = 150.0f;

	// 트레이스 채널 (기본: Visibility)
	UPROPERTY(EditAnywhere, Category = "Condition")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// 트레이스 구체 반지름 (캐릭터 폭 고려)
	UPROPERTY(EditAnywhere, Category = "Condition")
	float TraceRadius = 30.0f;
	
	// 디버그 드로잉 여부
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;
};
