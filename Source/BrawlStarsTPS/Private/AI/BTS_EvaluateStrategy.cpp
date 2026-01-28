// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTS_EvaluateStrategy.h"
#include "AI/BrawlAIController.h"
#include "BrawlCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"

UBTS_EvaluateStrategy::UBTS_EvaluateStrategy()
{
	NodeName = TEXT("Evaluate Strategy");
	
	// 기본 키 필터 설정
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_EvaluateStrategy, TargetActorKey), AActor::StaticClass());
	StrategyStateKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_EvaluateStrategy, StrategyStateKey), StaticEnum<EBrawlAIStrategy>());
	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_EvaluateStrategy, DistanceToTargetKey));
}

void UBTS_EvaluateStrategy::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ABrawlAIController* AIController = Cast<ABrawlAIController>(OwnerComp.GetAIOwner());
	ABrawlCharacter* MyPawn = AIController ? Cast<ABrawlCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AIController || !MyPawn || !Blackboard)
	{
		return;
	}

	// 1. 타겟 확인
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	
	if (!TargetActor)
	{
		// 타겟이 없으면 무조건 "순찰(Patrol)"
		Blackboard->SetValueAsEnum(StrategyStateKey.SelectedKeyName, (uint8)EBrawlAIStrategy::Patrol);
		return;
	}

	// 2. 데이터 가져오기 (설정값, 현재 상태)
	const FAICombatSettings& Settings = MyPawn->GetAICombatSettings();
	float Distance = MyPawn->GetDistanceTo(TargetActor);
	
	// 블랙보드에 거리 업데이트 (다른 데코레이터 등에서 쓸 수 있게)
	Blackboard->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);

	// 현재 체력 비율 계산
	float Health = 0.0f;
	float MaxHealth = 1.0f;
	if (const UBrawlAttributeSet* AttribSet = MyPawn->GetAbilitySystemComponent()->GetSet<UBrawlAttributeSet>())
	{
		Health = AttribSet->GetHealth();
		MaxHealth = AttribSet->GetMaxHealth();
	}
	float HealthRatio = (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;

	// 3. 현재 전략 상태 가져오기 (히스테리시스 처리를 위해)
	EBrawlAIStrategy CurrentStrategy = (EBrawlAIStrategy)Blackboard->GetValueAsEnum(StrategyStateKey.SelectedKeyName);
	EBrawlAIStrategy NewStrategy = CurrentStrategy;

	// 4. 전략 결정 로직

	// [도주(Flee) 판정]
	bool bShouldFlee = false;
	
	if (CurrentStrategy == EBrawlAIStrategy::Flee)
	{
		// 이미 도주 중이라면: 충분히 회복하고 적과 멀어져야 도주 해제 (Hysteresis)
		// 조건: (체력이 회복됨) AND (거리가 최소 거리보다 멀어짐)
		bool bRecovered = (HealthRatio >= Settings.ResumeCombatHealthRatio);
		bool bSafeDistance = (Distance > Settings.MinCombatRange * 1.5f); // 여유를 조금 둠

		if (!bRecovered && !bSafeDistance)
		{
			bShouldFlee = true; // 계속 도주
		}
		// 둘 다 만족하면 도주 해제 -> 아래 로직에서 Combat/Move 결정
	}
	else
	{
		// 도주 중이 아님: 진입 조건 검사
		// 조건: (체력이 낮음) OR (너무 가까움)
		if (HealthRatio <= Settings.FleeHealthRatio || Distance < Settings.MinCombatRange)
		{
			bShouldFlee = true;
		}
	}

	if (bShouldFlee)
	{
		NewStrategy = EBrawlAIStrategy::Flee;
	}
	else
	{
		// [이동(Move) vs 교전(Combat) 판정]
		
		// 최대 사거리보다 멀면 -> 이동
		if (Distance > Settings.MaxCombatRange)
		{
			NewStrategy = EBrawlAIStrategy::Move;
		}
		// 사거리 안이면 -> 교전 (접근은 교전 태스크 안에서 처리: Preferred Range까지)
		else
		{
			NewStrategy = EBrawlAIStrategy::Combat;
		}
	}

	// 5. 변경사항 적용
	if (NewStrategy != CurrentStrategy)
	{
		Blackboard->SetValueAsEnum(StrategyStateKey.SelectedKeyName, (uint8)NewStrategy);
		 UE_LOG(LogTemp, Log, TEXT("AI [%s] Strategy Changed: %s -> %s (Dist: %.0f, HP: %.2f)"), 
		 	*MyPawn->GetName(), 
		 	*UEnum::GetValueAsString(CurrentStrategy), 
		 	*UEnum::GetValueAsString(NewStrategy),
		 	Distance, HealthRatio);
	}
}
