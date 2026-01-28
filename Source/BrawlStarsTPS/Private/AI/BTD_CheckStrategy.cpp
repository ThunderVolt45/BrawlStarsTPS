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
}

bool UBTD_CheckStrategy::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return false;

	// 블랙보드에서 값 가져오기 (Enum/Byte는 uint8로 취급)
	uint8 CurrentValue = Blackboard->GetValueAsEnum(StrategyKey.SelectedKeyName);
	
	// 만약 키 타입이 Enum이 아니라 Byte/Int로 잡혀있을 경우를 대비해 GetValueAsInt 시도 가능하지만,
	// 보통 GetValueAsEnum이 uint8을 리턴하므로 호환됨.
	if (CurrentValue == 0 && Blackboard->GetKeyType(StrategyKey.GetSelectedKeyID()) != UBlackboardKeyType_Enum::StaticClass())
	{
		// Enum 타입이 아닐 경우 Integer나 Byte로 읽기 시도
		CurrentValue = (uint8)Blackboard->GetValueAsInt(StrategyKey.SelectedKeyName);
	}

	return CurrentValue == (uint8)TargetStrategy;
}
