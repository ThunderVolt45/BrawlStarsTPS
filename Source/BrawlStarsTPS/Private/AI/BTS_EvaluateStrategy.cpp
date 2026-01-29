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

	// 3. 현재 전략 상태 가져오기
	EBrawlAIStrategy CurrentStrategy = (EBrawlAIStrategy)Blackboard->GetValueAsEnum(StrategyStateKey.SelectedKeyName);
	EBrawlAIStrategy NewStrategy = CurrentStrategy;

	// 4. 전략 결정 로직
	// 4-1. 도주(Flee) 판정
	bool bShouldFlee = false;
	
	if (CurrentStrategy == EBrawlAIStrategy::Flee)
	{
		bShouldFlee = true;
		
		// 이미 도주 중이라면: 충분히 회복하고 적과 멀어져야 도주 해제 (Hysteresis)
		bool bRecovered = (HealthRatio >= Settings.ResumeCombatHealthRatio);
		bool bSafeDistance = (Distance > Settings.MinCombatRange * 1.5f); 

		if (bRecovered && bSafeDistance)
		{
			bShouldFlee = false;
		}
	}
	else
	{
		// 도주 중이 아님: 진입 조건 검사
		if (HealthRatio <= Settings.FleeHealthRatio || Distance < Settings.MinCombatRange)
		{
			bShouldFlee = true;
			NewStrategy = EBrawlAIStrategy::Flee;
		}
	}

	if (!bShouldFlee)
	{
		// [이동(Move) vs 교전(Combat) 판정]
		if (Distance > Settings.MaxCombatRange)
		{
			NewStrategy = EBrawlAIStrategy::Move;
		}
		else
		{
			NewStrategy = EBrawlAIStrategy::Combat;
		}
	}

	// 5. 변경사항 적용
	if (NewStrategy != CurrentStrategy)
	{
		Blackboard->SetValueAsEnum(StrategyStateKey.SelectedKeyName, (uint8)NewStrategy);
	}
}

void UBTS_EvaluateStrategy::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (APawn* MyPawn = AIController->GetPawn())
		{
			// 중복 바인딩 방지를 위해 Unique 사용
			MyPawn->OnTakeAnyDamage.AddUniqueDynamic(this, &UBTS_EvaluateStrategy::HandleTakeAnyDamage);
		}
	}
}

void UBTS_EvaluateStrategy::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (APawn* MyPawn = AIController->GetPawn())
		{
			MyPawn->OnTakeAnyDamage.RemoveDynamic(this, &UBTS_EvaluateStrategy::HandleTakeAnyDamage);
		}
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTS_EvaluateStrategy::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Error, TEXT("HandleTakeAnyDamage"));
	
	// 1. 유효성 검사
	ABrawlCharacter* MyBrawler = Cast<ABrawlCharacter>(DamagedActor);
	
	// 공격자가 없거나, 자기 자신이거나(자해), 유효하지 않은 경우 무시
	if (!MyBrawler || !DamageCauser || MyBrawler == DamageCauser)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(MyBrawler->GetController());
	if (!AIController)
	{
		return;
	}

	UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	// 2. 공격자를 타겟으로 즉시 지정
	Blackboard->SetValueAsObject(TargetActorKey.SelectedKeyName, DamageCauser);

	// 3. 긴급 상태 판단 (전투 or 도주)
	// 현재 체력 확인
	float Health = 0.0f;
	float MaxHealth = 1.0f;
	
	if (UAbilitySystemComponent* ASC = MyBrawler->GetAbilitySystemComponent())
	{
		if (const UBrawlAttributeSet* AttribSet = ASC->GetSet<UBrawlAttributeSet>())
		{
			Health = AttribSet->GetHealth();
			MaxHealth = AttribSet->GetMaxHealth();
		}
	}
	
	float HealthRatio = (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
	
	const FAICombatSettings& Settings = MyBrawler->GetAICombatSettings();

	EBrawlAIStrategy NewStrategy = EBrawlAIStrategy::Combat; // 기본적으로 반격

	if (HealthRatio <= Settings.FleeHealthRatio)
	{
		NewStrategy = EBrawlAIStrategy::Flee; // 체력 없으면 도주
	}

	// 현재 전략을 덮어씀 (즉시 행동 전환 유도)
	Blackboard->SetValueAsEnum(StrategyStateKey.SelectedKeyName, (uint8)NewStrategy);
}
