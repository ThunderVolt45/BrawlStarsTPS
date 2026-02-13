// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTD_CheckStrategy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"

UBTD_CheckStrategy::UBTD_CheckStrategy()
{
	NodeName = TEXT("Check Strategy");
	
	// 키 필터: Enum 또는 Byte (uint8)
	StrategyKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTD_CheckStrategy, StrategyKey), StaticEnum<EBrawlAIStrategy>());
	StrategyKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTD_CheckStrategy, StrategyKey));

	// 노드가 활성화/비활성화될 때 알림을 받도록 설정
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

bool UBTD_CheckStrategy::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return false;

	uint8 CurrentValue = Blackboard->GetValueAsEnum(StrategyKey.SelectedKeyName);
	
	if (CurrentValue == 0 && Blackboard->GetKeyType(StrategyKey.GetSelectedKeyID()) != UBlackboardKeyType_Enum::StaticClass())
	{
		CurrentValue = (uint8)Blackboard->GetValueAsInt(StrategyKey.SelectedKeyName);
	}

	return CurrentValue == (uint8)TargetStrategy;
}

void UBTD_CheckStrategy::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard)
	{
		// 블랙보드 키 변경 감시 등록
		Blackboard->RegisterObserver(StrategyKey.GetSelectedKeyID(), this, 
			FOnBlackboardChangeNotification::CreateUObject(this, &UBTD_CheckStrategy::OnBlackboardChange));
	}
}

void UBTD_CheckStrategy::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard)
	{
		// 감시 해제
		Blackboard->UnregisterObserversFrom(this);
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTD_CheckStrategy::OnBlackboardChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey KeyID)
{
	UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	if (!OwnerComp) return EBlackboardNotificationResult::RemoveObserver;

	// 전략 키 값이 변경되면 행동 트리에 즉시 실행 상태 재평가를 요청합니다.
	// 이를 통해 FlowAbortMode 설정에 따른 중단(Abortion)이 즉각적으로 이루어집니다.
	OwnerComp->RequestExecution(this);

	return EBlackboardNotificationResult::ContinueObserving;
}
