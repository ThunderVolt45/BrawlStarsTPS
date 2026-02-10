// Copyright Epic Games, Inc. All Rights Reserved.

#include "BrawlStarsTPSPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "UI/BrawlHUDWidget.h"
#include "Components/BrawlMatchFlowComponent.h"
#include "AbilitySystemInterface.h"
#include "BrawlCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

ABrawlStarsTPSPlayerController::ABrawlStarsTPSPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	// 매치 흐름 컴포넌트 생성
	MatchFlowComponent = CreateDefaultSubobject<UBrawlMatchFlowComponent>(TEXT("MatchFlowComponent"));
}

void ABrawlStarsTPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController()) return;
	
	if (!BrawlHUDClass) return;

	// 위젯이 없다면 새로 생성
	if (!BrawlHUDWidget)
	{
		BrawlHUDWidget = CreateWidget<UBrawlHUDWidget>(this, BrawlHUDClass);
		if (BrawlHUDWidget)
		{
			BrawlHUDWidget->AddToViewport();
		}
	}

	// Pawn이 있다면 연결 (이미 BeginPlay 시점에 Pawn이 있을 수 있음)
	if (GetPawn())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetPawn()))
		{
			if (BrawlHUDWidget)
			{
				BrawlHUDWidget->BindAttributeCallbacks(ASI->GetAbilitySystemComponent());

				if (ABrawlCharacter* BrawlChar = Cast<ABrawlCharacter>(GetPawn()))
				{
					BrawlHUDWidget->InitializeBrawlerUI(BrawlChar);
				}
			}
		}
	}
}

void ABrawlStarsTPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// 서버 로직: 필요한 경우 여기서 처리 (UI는 클라이언트 소관이므로 주로 여기선 스킵)
}

void ABrawlStarsTPSPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	// 클라이언트 로직: 로컬 플레이어가 Pawn을 빙의했을 때 호출됨
	if (IsLocalPlayerController() && BrawlHUDWidget)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(P))
		{
			BrawlHUDWidget->BindAttributeCallbacks(ASI->GetAbilitySystemComponent());

			if (ABrawlCharacter* BrawlChar = Cast<ABrawlCharacter>(P))
			{
				BrawlHUDWidget->InitializeBrawlerUI(BrawlChar);
			}
		}
	}
}

void ABrawlStarsTPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// only add IMCs for local player controllers
	if (!IsLocalPlayerController()) return;

	// Add Input Mapping Contexts
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void ABrawlStarsTPSPlayerController::ShowMatchStartUI_Implementation()
{
	if (IsLocalPlayerController() && MatchFlowComponent)
	{
		MatchFlowComponent->StartIntroSequence();
	}
}

void ABrawlStarsTPSPlayerController::ShowMatchResultUI_Implementation(bool bIsWinner, int32 Rank)
{
	UE_LOG(LogTemp, Warning, TEXT("PC: ShowMatchResultUI_Implementation RPC Received! Winner: %d, Rank: %d"), bIsWinner, Rank);

	if (IsLocalPlayerController())
	{
		if (MatchFlowComponent)
		{
			MatchFlowComponent->StartOutroSequence(bIsWinner, Rank);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PC: MatchFlowComponent is NULL on local PC!"));
		}
	}
}

void ABrawlStarsTPSPlayerController::PlayNoAmmoAnimation_Implementation()
{
	if (BrawlHUDWidget)
	{
		BrawlHUDWidget->PlayNoAmmoAnimation();
	}
}


void ABrawlStarsTPSPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// 로컬 플레이어인 경우에만 조준 보조 및 HUD 업데이트 수행
	if (IsLocalPlayerController())
	{
		// 1. 가장 적합한 타겟 탐색 (화면 중앙 기준)
		FindBestTarget();
		
		// 2. 타겟이 있다면 예측 사격을 고려하여 부드럽게 회전
		ApplyAimAssist(DeltaTime);
		
		// 3. HUD 요소 업데이트
		if (BrawlHUDWidget)
		{
			// HUD의 반경 설정을 컨트롤러에도 동기화
			AimDetectionRadius = BrawlHUDWidget->ReticleCircleRadius;

			ABrawlCharacter* Target = CurrentAimTarget.Get();
			
			// [중앙 리틱클] 타겟 유무에 따라 색상 변경
			if (BrawlHUDWidget->ReticleCircle)
			{
				FLinearColor ReticleColor = Target ? FLinearColor::Red : FLinearColor::White;
				BrawlHUDWidget->ReticleCircle->SetColorAndOpacity(ReticleColor);
			}

			// [타겟 추적 이미지]
			if (BrawlHUDWidget->TargetIndicator)
			{
				if (Target)
				{
					FVector2D ScreenPos;
					// 타겟의 정중앙(Capsule Center) 위치를 사용
					FVector TargetWorldPos = Target->GetActorLocation();
					
					// 월드 좌표를 HUD 위젯 좌표계로 변환
					if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(this, TargetWorldPos, ScreenPos, false))
					{
						BrawlHUDWidget->TargetIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
						
						if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(BrawlHUDWidget->TargetIndicator))
						{
							// 위치 설정 및 중앙 정렬
							CanvasSlot->SetPosition(ScreenPos);
							CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
							
							if (CanvasSlot->GetSize().IsZero())
							{
								CanvasSlot->SetSize(FVector2D(64.0f, 64.0f));
							}
						}
					}
					else
					{
						BrawlHUDWidget->TargetIndicator->SetVisibility(ESlateVisibility::Collapsed);
					}
				}
				else
				{
					BrawlHUDWidget->TargetIndicator->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
			else if (Target)
			{
				// 바인딩 문제 로그
				static bool bWarnedOnce = false;
				if (!bWarnedOnce)
				{
					UE_LOG(LogTemp, Warning, TEXT("Target detected but 'TargetIndicator' widget is NULL! Check WBP_BrawlHUD."));
					bWarnedOnce = true;
				}
			}
		}
	}
}

void ABrawlStarsTPSPlayerController::FindBestTarget()
{
	ABrawlCharacter* MyChar = Cast<ABrawlCharacter>(GetPawn());
	if (!MyChar)
	{
		CurrentAimTarget = nullptr;
		return;
	}

	// 최대 사거리 계산 (발사체 속도 * 수명)
	float MaxWorldRange = MyChar->GetEstimatedProjectileSpeed() * MyChar->GetEstimatedProjectileLifetime();
	float MaxWorldRangeSq = FMath::Square(MaxWorldRange);

	ABrawlCharacter* BestTarget = nullptr;
	float BestDistSq = FMath::Square(AimDetectionRadius);
	
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);
	FVector2D ScreenCenter(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);

	for (TActorIterator<ABrawlCharacter> It(GetWorld()); It; ++It)
	{
		ABrawlCharacter* OtherChar = *It;
		if (!OtherChar || OtherChar == MyChar || OtherChar->IsDead()) continue;

		// 같은 팀 제외 (팀 ID가 255가 아니면서 같으면)
		if (MyChar->GetTeamID() != 255 && MyChar->GetTeamID() == OtherChar->GetTeamID()) continue;

		// 시야/은신 확인 (IsVisibleTo 사용)
		if (!OtherChar->IsVisibleTo(MyChar->GetGenericTeamId())) continue;

		// 월드 거리 확인 (최대 사거리 제한)
		float WorldDistSq = FVector::DistSquared(MyChar->GetActorLocation(), OtherChar->GetActorLocation());
		if (WorldDistSq > MaxWorldRangeSq) continue;

		// 장애물 체크 (Line of Sight)
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyChar);
		QueryParams.AddIgnoredActor(OtherChar);
		
		// 캐릭터의 중심(Capsule Center)끼리 레이캐스트 수행
		FVector TraceStart = MyChar->GetActorLocation();
		FVector TraceEnd = OtherChar->GetActorLocation();
		
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			// 무언가에 걸렸다면 (장애물이 있다면) 스킵
			continue;
		}

		FVector2D ScreenPos;
		
		// 화면에 렌더링 가능한지(앞에 있는지) 확인하고 현재 목표가 기존 목표보다 더 가까이에 있다면 목표를 변경한다
		if (ProjectWorldLocationToScreen(OtherChar->GetActorLocation(), ScreenPos))
		{
			float DistSq = FVector2D::DistSquared(ScreenPos, ScreenCenter);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestTarget = OtherChar;
			}
		}
	}

	CurrentAimTarget = BestTarget;
}

void ABrawlStarsTPSPlayerController::ApplyAimAssist(float DeltaTime)
{
	ABrawlCharacter* MyChar = Cast<ABrawlCharacter>(GetPawn());
	ABrawlCharacter* Target = CurrentAimTarget.Get();

	if (!MyChar || !Target || !PlayerCameraManager)
	{
		PredictedAimLocation = FVector::ZeroVector;
		return;
	}

	// 1. 유효성 및 사거리 확인
	float MaxWorldRange = MyChar->GetEstimatedProjectileSpeed() * MyChar->GetEstimatedProjectileLifetime();
	float DistToTarget = FVector::Dist(MyChar->GetActorLocation(), Target->GetActorLocation());

	if (Target->IsDead() || DistToTarget > MaxWorldRange)
	{
		CurrentAimTarget = nullptr;
		PredictedAimLocation = FVector::ZeroVector;
		return;
	}

	// 2. 장애물 체크 (조준 유지 중 벽 뒤로 숨었는지 확인)
	{
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyChar);
		QueryParams.AddIgnoredActor(Target);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, MyChar->GetActorLocation(), Target->GetActorLocation(), ECC_Visibility, QueryParams))
		{
			CurrentAimTarget = nullptr;
			PredictedAimLocation = FVector::ZeroVector;
			return;
		}
	}

	// 3. 위치 및 속도 정보 가져오기
	FVector CameraLoc = PlayerCameraManager->GetCameraLocation();
	FVector MyLoc = MyChar->GetActorLocation();
	FVector TargetLoc = Target->GetActorLocation();
	FVector TargetVel = Target->GetVelocity();
	float ProjectileSpeed = MyChar->GetEstimatedProjectileSpeed();

	// 3. 예측 사격 지점 계산 (Linear Prediction)
	// 발사 지점(캐릭터 위치 근처)에서 목표까지의 거리를 기준으로 탄착 시간 계산
	float TimeToHit = (ProjectileSpeed > 0.f) ? (DistToTarget / ProjectileSpeed) : 0.f;

	// 목표의 미래 위치 예측
	PredictedAimLocation = TargetLoc + (TargetVel * TimeToHit);

	// 4. 회전값 계산
	// 기준점을 MyChar->GetActorLocation()이 아닌 CameraLoc으로 변경하여 
	// 화면 정중앙(리틱클)에 목표가 오도록 함
	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(CameraLoc, PredictedAimLocation);
	FRotator CurrentRot = GetControlRotation();

	// 5. 부드럽게 회전 (Interp)
	FRotator NewRot = FMath::RInterpTo(CurrentRot, LookAtRot, DeltaTime, AimAssistInterpSpeed);

	// 화면 기울어짐 방지를 위해 Roll 값을 강제로 0으로 고정
	NewRot.Roll = 0.0f;

	// 컨트롤러 회전 적용 (카메라와 캐릭터가 함께 회전)
	SetControlRotation(NewRot);
}