// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTS_EvaluateStrategy.h"
#include "AI/BrawlAIController.h"
#include "AI/BrawlAIStrategy.h" // 추가
#include "BrawlCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"

UBTS_EvaluateStrategy::UBTS_EvaluateStrategy()
{
	NodeName = TEXT("Evaluate Strategy");
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	// 기본 키 필터 설정
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_EvaluateStrategy, TargetActorKey), AActor::StaticClass());
	
	// Enum 필터 복구
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

	// 사망 시 행동 트리 정지
	if (MyPawn->IsDead())
	{
		OwnerComp.StopTree(EBTStopMode::Safe);
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
	
	// 블랙보드에 거리 업데이트
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

	// 타겟 체력 비율 계산
	float TargetHealthRatio = 1.0f;
	bool bIsCombatant = false; // 실제로 위협이 되는 대상(브롤러, 소환물 등)인가?

	if (const ABrawlCharacter* TargetBrawlChar = Cast<ABrawlCharacter>(TargetActor))
	{
		// Hero(브롤러)나 Summon(소환물)인 경우에만 교전 대상으로 간주하고 도주를 고려함
		EBrawlCharacterType TargetType = TargetBrawlChar->GetCharacterType();
		if (TargetType == EBrawlCharacterType::Hero || TargetType == EBrawlCharacterType::Summon)
		{
			bIsCombatant = true;
		}

		if (UAbilitySystemComponent* TargetASC = TargetBrawlChar->GetAbilitySystemComponent())
		{
			if (const UBrawlAttributeSet* TargetAttribSet = TargetASC->GetSet<UBrawlAttributeSet>())
			{
				float TargetHealth = TargetAttribSet->GetHealth();
				float TargetMaxHealth = TargetAttribSet->GetMaxHealth();
				TargetHealthRatio = (TargetMaxHealth > 0.0f) ? (TargetHealth / TargetMaxHealth) : 0.0f;
			}
		}
	}

	// 3. 현재 전략 상태 가져오기
	EBrawlAIStrategy CurrentStrategy = (EBrawlAIStrategy)Blackboard->GetValueAsEnum(StrategyStateKey.SelectedKeyName);
	
	// 타겟이 있으므로 더 이상 Patrol 상태는 불가능함 (기본값 설정)
	EBrawlAIStrategy NewStrategy = EBrawlAIStrategy::Combat;

	// 4. 전략 결정 로직
	// 4-1. 도주(Flee) 여부 판정
	bool bShouldFlee = false;
	
	// 오직 위협적인 대상(Hero, Summon)을 상대로 할 때만 도주를 고려함 (상자 등 Etc 타입은 무시)
	if (bIsCombatant)
	{
		if (CurrentStrategy == EBrawlAIStrategy::Flee)
		{
			// 이미 도주 중이라면: 충분히 회복하고 적과 멀어지거나 시야가 차단되어야 도주 해제
			bool bRecovered = (HealthRatio >= Settings.ResumeCombatHealthRatio);
			bool bSafeDistance = (Distance > Settings.MinCombatRange * 1.5f); 

			bool bLineOfSight = true;
			{
				FVector Start = MyPawn->GetPawnViewLocation();
				FVector End = TargetActor->GetActorLocation();
				FHitResult HitResult;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(MyPawn);

				if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel, Params))
				{
					if (HitResult.GetActor() != TargetActor)
					{
						bLineOfSight = false; // 장애물에 가려짐
					}
				}
			}

			// 아직 위험한 상황(시야 확보됨 & 미회복)이라면 도주 유지
			if (!((bRecovered && bSafeDistance) || !bLineOfSight))
			{
				bShouldFlee = true;
			}
		}
		else
		{
			// 도주 중이 아님: 진입 조건 검사
			bool bTargetIsLow = (TargetHealthRatio <= Settings.PursuitTargetHealthRatio);
			bool bMyHealthIsLow = (HealthRatio <= Settings.FleeHealthRatio);
			bool bTooClose = (Distance < Settings.MinCombatRange);

			// (내 체력 낮음 & 상대 체력 안 낮음) 혹은 (너무 가까움) 이면 도주
			if ((bMyHealthIsLow && !bTargetIsLow) || bTooClose)
			{
				bShouldFlee = true;
			}
		}
	}

	// 4-2. 최종 전략 확정
	if (bShouldFlee)
	{
		NewStrategy = EBrawlAIStrategy::Flee;
	}
	else if (Distance > Settings.MaxCombatRange)
	{
		// 사거리 밖이면 이동
		NewStrategy = EBrawlAIStrategy::Move;
	}
	else
	{
		// 사거리 안이면 교전
		NewStrategy = EBrawlAIStrategy::Combat;
	}

	// 5. 변경사항 적용
	if (NewStrategy != CurrentStrategy)
	{
		Blackboard->SetValueAsEnum(StrategyStateKey.SelectedKeyName, (uint8)NewStrategy);
	}
}