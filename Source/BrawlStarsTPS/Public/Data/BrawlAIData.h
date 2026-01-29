// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BrawlAIData.generated.h"

/**
 * FBrawlAIData
 * 
 * 브롤러별 AI 행동 패턴과 설정을 정의하는 데이터 테이블 구조체입니다.
 * DT_BrawlerAI 테이블에서 사용됩니다.
 */
USTRUCT(BlueprintType)
struct BRAWLSTARSTPS_API FBrawlAIData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// AI가 사용할 전투 행동 트리 (서브 트리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
	TObjectPtr<UBehaviorTree> CombatBehaviorTree;

	// 최대 교전 거리 (이보다 멀면 "이동" 전략)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float MaxCombatRange = 1000.0f;

	// 선호 교전 거리 (이 거리 유지를 위해 이동/후퇴)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float PreferredCombatRange = 700.0f;

	// 최소 교전 거리 (이보다 가까우면 "도주" 전략)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float MinCombatRange = 300.0f;

	// 도주 시작 체력 비율 (0.0 ~ 1.0) - 이 이하로 떨어지면 도주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Survival")
	float FleeHealthRatio = 0.3f;

	// 도주 종료(복귀) 체력 비율 (0.0 ~ 1.0) - 이 이상 회복되면 다시 교전
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Survival")
	float ResumeCombatHealthRatio = 0.7f;
};
