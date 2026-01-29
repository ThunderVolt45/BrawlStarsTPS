// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BrawlAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BrawlCharacter.h"

ABrawlAIController::ABrawlAIController()
{
	// 1. Behavior Tree & Blackboard 컴포넌트 생성
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	// 2. Perception 컴포넌트 생성
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	// 3. 시각 설정 생성
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	
	if (SightConfig)
	{
		SightConfig->SightRadius = 1500.0f; // 시야 거리
		SightConfig->LoseSightRadius = 1800.0f; // 시야 상실 거리
		SightConfig->PeripheralVisionAngleDegrees = 180.0f; // 시야각 (360도 감지 하려면 180 입력)
		
		// 감지 대상 설정: 적, 중립, 아군 모두 감지
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		// 설정 등록
		AIPerception->ConfigureSense(*SightConfig);
		AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	}
}

void ABrawlAIController::BeginPlay()
{
	Super::BeginPlay();

	// 감지 이벤트 바인딩
	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ABrawlAIController::OnTargetDetected);
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

void ABrawlAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// 감각 자극이 "성공(감지됨)"인지 "실패(사라짐)"인지 확인
	bool bDetected = Stimulus.WasSuccessfullySensed();

	if (bDetected)
	{
		// 적군인지 확인
		if (GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile)
		{
			SetFocus(Actor);
			UpdateTargetInBlackboard(Actor);
			UE_LOG(LogTemp, Log, TEXT("AI [%s] Detected Enemy: %s"), *GetName(), *Actor->GetName());
		}
	}
	else
	{
		// 시야에서 사라짐 (추후 로직: 마지막 위치 기억 등)
		// 현재는 단순히 타겟 해제 또는 거리 기반 로직에 맡김
		SetFocus(nullptr);
		UpdateTargetInBlackboard(nullptr);
		UE_LOG(LogTemp, Log, TEXT("AI [%s] Lost Sight of: %s"), *GetName(), *Actor->GetName());
	}
}

void ABrawlAIController::UpdateTargetInBlackboard(AActor* TargetActor)
{
	if (BlackboardComponent)
	{
		// 블랙보드 키 이름 "TargetActor"에 저장 (블랙보드 에셋 생성 시 맞춰야 함)
		BlackboardComponent->SetValueAsObject(FName("TargetActor"), TargetActor);
	}
}
