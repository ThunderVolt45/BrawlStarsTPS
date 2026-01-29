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
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "BrawlAttributeSet.h"

// ... (기존 코드)

void ABrawlAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 타겟 목록이 있을 때 최적의 타겟 재선정
	if (DetectedEnemies.Num() > 0)
	{
		AActor* BestTarget = SelectBestTarget();
		AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));

		// 타겟이 바뀌었거나, 현재 타겟이 없는데 베스트 타겟이 생긴 경우
		if (BestTarget != CurrentTarget)
		{
			// 너무 잦은 교체를 막기 위해 일정 조건(예: 점수 차이)을 둘 수도 있음
			// 여기서는 즉시 반영
			UpdateTargetInBlackboard(BestTarget);
			SetFocus(BestTarget);
		}
	}
}

AActor* ABrawlAIController::SelectBestTarget()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return nullptr;

	AActor* BestActor = nullptr;
	float BestScore = -100.0f;

	// 현재 타겟 확인 (가산점 부여용)
	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));

	// 목록 순회 (유효하지 않은 액터는 정리)
	for (auto It = DetectedEnemies.CreateIterator(); It; ++It)
	{
		AActor* Enemy = *It;
		if (!Enemy || !Enemy->IsValidLowLevel() || Enemy->IsActorBeingDestroyed())
		{
			It.RemoveCurrent();
			continue;
		}

		// 점수 계산 (높을수록 좋음)
		float Distance = MyPawn->GetDistanceTo(Enemy);
		
		// 1. 거리 점수 (가까울수록 높음, 최대 2000cm 기준)
		// 2000cm일 때 0점, 0cm일 때 100점
		float DistanceScore = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f) * 100.0f;

		// 2. 현재 타겟 유지 점수 (타겟이 자꾸 바뀌는 것 방지, 히스테리시스)
		float MaintainScore = (Enemy == CurrentTarget) ? 20.0f : 0.0f;

		// 3. 타겟의 체력 점수 (낮을수록 높음, 최대 50점)
		float HealthScore = 0.0f;
		if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Enemy))
		{
			if (UAbilitySystemComponent* EnemyASC = ASCInterface->GetAbilitySystemComponent())
			{
				bool bFoundHealth = false;
				float Health = EnemyASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetHealthAttribute(), bFoundHealth);
				float MaxHealth = EnemyASC->GetGameplayAttributeValue(UBrawlAttributeSet::GetMaxHealthAttribute(), bFoundHealth);

				if (bFoundHealth && MaxHealth > 0.0f)
				{
					float HealthRatio = FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
					// 체력이 낮을 수록 50점에 수렴
					HealthScore = (1.0f - HealthRatio) * 50.0f;
				}
			}
		}

		// 최종 판정
		float TotalScore = DistanceScore + MaintainScore + HealthScore;
		
		if (TotalScore > BestScore)
		{
			BestScore = TotalScore;
			BestActor = Enemy;
		}
	}

	return BestActor;
}

void ABrawlAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// 자신이거나 이미 사망한 대상 무시
	if (!Actor || !Actor->IsValidLowLevel()) return;

	// 적군인지 확인 (아군이면 무시, 중립/적군은 통과)
	if (GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Friendly) return;

	// 1. 피격 감지 (Damage) - 즉시 리스트 추가 및 강제 타겟팅
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			DetectedEnemies.Add(Actor); // 리스트 추가
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ForgetTarget);
			
			// 피격당하면 거리 상관없이 즉시 해당 적을 본다
			SetFocus(Actor);
			UpdateTargetInBlackboard(Actor);
			UE_LOG(LogTemp, Warning, TEXT("AI [%s] HIT BY [%s]! Added to List & Switching Target."), *GetName(), *Actor->GetName());
		}
		return;
	}

	// 2. 시각 감지 (Sight)
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// 적 발견: 리스트 추가
			DetectedEnemies.Add(Actor);
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ForgetTarget);

			// 리스트에 하나뿐이면 즉시 타겟팅
			if (DetectedEnemies.Num() == 1)
			{
				UpdateTargetInBlackboard(Actor);
				SetFocus(Actor);
			}
		}
		else
		{
			// 시야에서 사라짐: 리스트에서 당장 제거하지 않음 (기억 유지)
			// 다만 현재 타겟이었다면 망각 타이머 시작
			AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
			if (CurrentTarget == Actor)
			{
				UE_LOG(LogTemp, Log, TEXT("AI [%s] Lost Sight of Target: %s. Starting Forget Timer (5.0s)."), *GetName(), *Actor->GetName());
				GetWorld()->GetTimerManager().SetTimer(TimerHandle_ForgetTarget, this, &ABrawlAIController::ForceForgetTarget, 5.0f, false);
			}
		}
	}
}

void ABrawlAIController::OnTargetForgotten(AActor* Actor)
{
	// 기억에서 완전히 사라짐 -> 리스트에서 제거
	DetectedEnemies.Remove(Actor);

	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
	if (CurrentTarget == Actor)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ForgetTarget); // 타이머 정리
		
		// 잊었지만 다른 적이 리스트에 남아있다면 교체 시도 (Tick에서 처리되겠지만 즉시 반영)
		if (DetectedEnemies.Num() > 0)
		{
			AActor* NextBest = SelectBestTarget();
			if (NextBest)
			{
				UpdateTargetInBlackboard(NextBest);
				SetFocus(NextBest);
				return;
			}
		}

		// 남은 적도 없으면 완전 해제
		SetFocus(nullptr);
		UpdateTargetInBlackboard(nullptr);
		UE_LOG(LogTemp, Log, TEXT("AI [%s] Forgotten Target: %s (List Empty)"), *GetName(), *Actor->GetName());
	}
}

void ABrawlAIController::ForceForgetTarget()
{
	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
	if (CurrentTarget)
	{
		// 강제 망각 시 리스트에서도 제거하는 정책 사용 (재발견 시 다시 추가됨)
		DetectedEnemies.Remove(CurrentTarget);
		
		// 남은 적이 있으면 교체
		if (DetectedEnemies.Num() > 0)
		{
			AActor* NextBest = SelectBestTarget();
			if (NextBest)
			{
				UpdateTargetInBlackboard(NextBest);
				SetFocus(NextBest);
				return;
			}
		}

		SetFocus(nullptr);
		UpdateTargetInBlackboard(nullptr);
		UE_LOG(LogTemp, Log, TEXT("AI [%s] Force Forgot Target (Timer Expired)."), *GetName());
	}
}

void ABrawlAIController::UpdateTargetInBlackboard(AActor* TargetActor)
{
	if (BlackboardComponent)
	{
		// 로그 너무 많이 남으면 주석 처리
		// UE_LOG(LogTemp, Log, TEXT("AI [%s] Target Updated: %s"), *GetName(), TargetActor ? *TargetActor->GetName() : TEXT("None"));
		BlackboardComponent->SetValueAsObject(FName("TargetActor"), TargetActor);
	}
}
