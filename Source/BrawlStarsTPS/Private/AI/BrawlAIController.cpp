// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BrawlAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h" // 추가
#include "BrawlCharacter.h"
#include "Math/UnitConversion.h"

ABrawlAIController::ABrawlAIController()
{
	// 1. Behavior Tree & Blackboard 컴포넌트 생성
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	
	// AIPerceptionComponent는 블루프린트에서 추가(Add Component)하여 사용합니다.
}

void ABrawlAIController::BeginPlay()
{
	Super::BeginPlay();

	// 블루프린트에서 추가한 AIPerceptionComponent 가져오기
	UAIPerceptionComponent* PerceptionComp = GetPerceptionComponent();
	
	if (PerceptionComp)
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABrawlAIController::OnTargetDetected);
		PerceptionComp->OnTargetPerceptionForgotten.AddDynamic(this, &ABrawlAIController::OnTargetForgotten);
		
		UE_LOG(LogTemp, Log, TEXT("AI [%s] Perception Component Found & Bound."), *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AI [%s] No Perception Component Found! Add one in Blueprint."), *GetName());
	}
}

void ABrawlAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Behavior Tree 실행
	if (DefaultBehaviorTree)
	{
		if (DefaultBehaviorTree->BlackboardAsset)
		{
			BlackboardComponent->InitializeBlackboard(*DefaultBehaviorTree->BlackboardAsset);
			
			// 브롤러별 고유 전투 트리(Combat Tree) 설정
			if (ABrawlCharacter* BrawlPawn = Cast<ABrawlCharacter>(InPawn))
			{
				if (UBehaviorTree* CombatTree = BrawlPawn->GetCombatBehaviorTree())
				{
					BlackboardComponent->SetValueAsObject(FName("CombatTree"), CombatTree);
					UE_LOG(LogTemp, Log, TEXT("AI [%s] Loaded Combat Tree: %s"), *GetName(), *CombatTree->GetName());
				}
			}

			BehaviorTreeComponent->StartTree(*DefaultBehaviorTree);
		}
	}
}

FGenericTeamId ABrawlAIController::GetGenericTeamId() const
{
	// 컨트롤러가 빙의한 Pawn의 팀 ID를 반환
	if (ABrawlCharacter* BrawlPawn = Cast<ABrawlCharacter>(GetPawn()))
	{
		return BrawlPawn->GetGenericTeamId();
	}

	return FGenericTeamId(255); // No Team
}

ETeamAttitude::Type ABrawlAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	// 상대방이 TeamAgentInterface를 구현하는지 확인
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	
	if (!OtherTeamAgent)
	{
		return ETeamAttitude::Neutral;
	}

	FGenericTeamId MyTeamID = GetGenericTeamId();
	FGenericTeamId OtherTeamID = OtherTeamAgent->GetGenericTeamId();

	// 팀 ID가 255(NoTeam)이면 모두 적대
	if (MyTeamID.GetId() == 255 || OtherTeamID.GetId() == 255)
	{
		return ETeamAttitude::Hostile;
	}

	// 같은 팀이면 우호, 다르면 적대
	return (MyTeamID == OtherTeamID) ? ETeamAttitude::Friendly : ETeamAttitude::Hostile;
}

#include "TimerManager.h" // 추가

// ... (기존 코드)

void ABrawlAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// 자신이거나 이미 사망한 대상 무시
	if (!Actor || !Actor->IsValidLowLevel()) return;

	// 적군인지 확인
	if (GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Friendly) return;

	// 1. 피격 감지 (Damage) - 즉시 타겟 변경 및 타이머 취소
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// 피격 시에는 망각 타이머 즉시 취소 (추격해야 하므로)
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ForgetTarget);
			
			SetFocus(Actor);
			UpdateTargetInBlackboard(Actor);
			UE_LOG(LogTemp, Warning, TEXT("AI [%s] HIT BY [%s]! Switching Target instantly."), *GetName(), *Actor->GetName());
		}
		return;
	}

	// 2. 시각 감지 (Sight)
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// 다시 보이면 망각 타이머 취소
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ForgetTarget);

			// 새로운 적 발견 -> 현재 타겟이 없으면 설정
			AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
			if (!CurrentTarget)
			{
				SetFocus(Actor);
				UpdateTargetInBlackboard(Actor);
				UE_LOG(LogTemp, Log, TEXT("AI [%s] Saw Enemy: %s"), *GetName(), *Actor->GetName());
			}
			else if (CurrentTarget != Actor)
			{
				// 이미 타겟이 있을 경우 새 타겟이 더 가깝다면 목표를 교체한다
				float DistToCurrent = GetPawn()->GetDistanceTo(CurrentTarget);
				float DistToNew = GetPawn()->GetDistanceTo(Actor);
				
				// 25% 이상 더 가까우면 교체
				if (DistToNew < DistToCurrent * 0.75f)
				{
					UpdateTargetInBlackboard(Actor);
				}
			}
		}
		else
		{
			// 시야에서 사라짐 (Lost Sight)
			// Perception Config의 MaxAge에 의존하지 않고, 직접 타이머를 돌려 확실하게 잊게 함
			AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
			if (CurrentTarget == Actor)
			{
				UE_LOG(LogTemp, Log, TEXT("AI [%s] Lost Sight of Target: %s. Starting Forget Timer (5.0s)."), *GetName(), *Actor->GetName());
				
				// 5초 뒤에 강제로 잊음
				GetWorld()->GetTimerManager().SetTimer(TimerHandle_ForgetTarget, this, &ABrawlAIController::ForceForgetTarget, 5.0f, false);
			}
		}
	}
}

void ABrawlAIController::OnTargetForgotten(AActor* Actor)
{
	UE_LOG(LogTemp, Log, TEXT("AI [%s] Perception System Try to Forgot Target: %s"), *GetName(), *Actor->GetName());
	
	// Perception System에 의해 잊혀짐 (MaxAge 만료)
	// 타이머보다 먼저 호출될 수도 있음
	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
	if (CurrentTarget == Actor)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ForgetTarget); // 타이머 정리
		SetFocus(nullptr);
		UpdateTargetInBlackboard(nullptr);
		UE_LOG(LogTemp, Log, TEXT("AI [%s] Perception System Forgot Target: %s"), *GetName(), *Actor->GetName());
	}
}

void ABrawlAIController::ForceForgetTarget()
{
	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
	if (CurrentTarget)
	{
		SetFocus(nullptr);
		UpdateTargetInBlackboard(nullptr);
		UE_LOG(LogTemp, Log, TEXT("AI [%s] Force Forgot Target (Timer Expired)."), *GetName());
	}
}

void ABrawlAIController::UpdateTargetInBlackboard(AActor* TargetActor)
{
	if (BlackboardComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("AI [%s] Perception System Update Target To: %s"), *GetName(), 
			TargetActor ? *TargetActor->GetName() : TEXT("null"));
		
		// 블랙보드 키 이름 "TargetActor"에 저장 (블랙보드 에셋 생성 시 맞춰야 함)
		BlackboardComponent->SetValueAsObject(FName("TargetActor"), TargetActor);
	}
}
