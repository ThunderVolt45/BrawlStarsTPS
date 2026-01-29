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

	// 목록 순회 (TMap Iteration)
	for (auto It = DetectedEnemies.CreateIterator(); It; ++It)
	{
		AActor* Enemy = It->Key;
		if (!Enemy || !Enemy->IsValidLowLevel() || Enemy->IsActorBeingDestroyed())
		{
			// 유효하지 않은 액터는 타이머 정리 후 제거
			GetWorld()->GetTimerManager().ClearTimer(It->Value);
			It.RemoveCurrent();
			continue;
		}

		// 점수 계산 (높을수록 좋음)
		float Distance = MyPawn->GetDistanceTo(Enemy);
		float DistanceScore = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f) * 100.0f;
		float MaintainScore = (Enemy == CurrentTarget) ? 20.0f : 0.0f;

		// 타겟의 체력 점수
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
					HealthScore = (1.0f - HealthRatio) * 50.0f;
				}
			}
		}

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
	if (!Actor || !Actor->IsValidLowLevel()) return;
	if (GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Friendly) return;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	// 1. 피격 감지 (Damage)
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// 리스트에 추가 (없으면 추가, 있으면 유지)
			if (!DetectedEnemies.Contains(Actor))
			{
				DetectedEnemies.Add(Actor, FTimerHandle());
			}

			// 해당 적의 망각 타이머가 있다면 취소 (추격해야 하므로)
			if (DetectedEnemies[Actor].IsValid())
			{
				TimerManager.ClearTimer(DetectedEnemies[Actor]);
			}
			
			// 즉시 타겟팅
			SetFocus(Actor);
			UpdateTargetInBlackboard(Actor);
			// UE_LOG(LogTemp, Warning, TEXT("AI [%s] HIT BY [%s]! Added to List & Switching Target."), *GetName(), *Actor->GetName());
		}
		return;
	}

	// 2. 시각 감지 (Sight)
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// 적 발견/재발견
			if (!DetectedEnemies.Contains(Actor))
			{
				DetectedEnemies.Add(Actor, FTimerHandle());
			}
			
			// 다시 보였으므로 타이머 취소
			if (DetectedEnemies[Actor].IsValid())
			{
				TimerManager.ClearTimer(DetectedEnemies[Actor]);
			}

			// 리스트에 하나뿐이면 즉시 타겟팅
			if (DetectedEnemies.Num() == 1)
			{
				UpdateTargetInBlackboard(Actor);
				SetFocus(Actor);
			}
		}
		else
		{
			// 시야에서 사라짐 -> 개별 타이머 시작
			if (DetectedEnemies.Contains(Actor))
			{
				// 이미 돌고 있는 타이머가 있다면 초기화하고 다시 시작
				TimerManager.ClearTimer(DetectedEnemies[Actor]);
				
				FTimerDelegate TimerDel;
				TimerDel.BindUObject(this, &ABrawlAIController::ForceForgetTarget, Actor);
				
				// 5초 뒤에 이 특정 적을 잊음
				TimerManager.SetTimer(DetectedEnemies[Actor], TimerDel, 5.0f, false);
				
				UE_LOG(LogTemp, Log, TEXT("AI [%s] Lost Sight of [%s]. Forget Timer Started (5.0s)."), *GetName(), *Actor->GetName());
			}
		}
	}
}

void ABrawlAIController::OnTargetForgotten(AActor* Actor)
{
	// Perception System에 의해 완전히 잊혀짐
	if (DetectedEnemies.Contains(Actor))
	{
		GetWorld()->GetTimerManager().ClearTimer(DetectedEnemies[Actor]);
		DetectedEnemies.Remove(Actor);
	}

	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
	if (CurrentTarget == Actor)
	{
		// 잊었지만 다른 적이 리스트에 남아있다면 교체 시도
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
	}
}

void ABrawlAIController::ForceForgetTarget(AActor* TargetToForget)
{
	// 타이머 만료로 인한 강제 망각
	if (DetectedEnemies.Contains(TargetToForget))
	{
		// 맵에서 제거 (타이머 핸들은 이미 만료되었으므로 Clear 불필요하지만 안전하게)
		GetWorld()->GetTimerManager().ClearTimer(DetectedEnemies[TargetToForget]);
		DetectedEnemies.Remove(TargetToForget);
		
		UE_LOG(LogTemp, Log, TEXT("AI [%s] Force Forgot [%s] (Timer Expired)."), *GetName(), *TargetToForget->GetName());
	}

	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
	if (CurrentTarget == TargetToForget)
	{
		// 현재 타겟을 잊었다면 다른 타겟 찾기
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

		UpdateTargetInBlackboard(nullptr);
		SetFocus(nullptr);
	}
}

void ABrawlAIController::UpdateTargetInBlackboard(AActor* TargetActor)
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsObject(FName("TargetActor"), TargetActor);
	}
}
