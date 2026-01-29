// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AI/BrawlAIStrategy.h" // 추가
#include "BrawlAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * ABrawlAIController
 * 
 * 브롤스타즈 스타일 AI 컨트롤러
 * - 시야(Sight) 감지를 통해 적을 탐지
 * - Behavior Tree를 실행하여 전투/이동 로직 수행
 * - IGenericTeamAgentInterface를 직접 상속받지 않고 Pawn(Character)의 인터페이스를 활용함
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABrawlAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
	
	// 감지 업데이트 델리게이트
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
	
	UFUNCTION()
	void OnTargetForgotten(AActor* Actor);

public:
	// 팀 ID 반환 (Pawn의 TeamID를 따라감)
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	// 행동 트리 컴포넌트 (Blackboard 포함)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	// 기본 행동 트리 에셋 (BP에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class UBehaviorTree> DefaultBehaviorTree;

	// 감지된 타겟 관리
	void UpdateTargetInBlackboard(AActor* TargetActor);

	// 타겟 망각 타이머
	FTimerHandle TimerHandle_ForgetTarget;

	// 타이머에 의해 호출될 함수
	void ForceForgetTarget();
};
