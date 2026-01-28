// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlBush.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "BrawlCharacter.h"

ABrawlBush::ABrawlBush()
{
	PrimaryActorTick.bCanEverTick = true;

	// 수풀 메시는 "은신 영역"으로 사용
	if (MeshComponent)
	{
		MeshComponent->SetCollisionProfileName(FName("OverlapAll"));
		MeshComponent->SetGenerateOverlapEvents(true);
	}

	// 접근 감지용 스피어 생성
	ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
	ProximitySphere->SetupAttachment(RootComponent);
	ProximitySphere->SetSphereRadius(ProximityRadius);
	ProximitySphere->SetCollisionProfileName(FName("OverlapAllDynamic")); // 동적 오버랩만 감지
	ProximitySphere->SetGenerateOverlapEvents(true);

	// 수풀은 파괴 가능함 (기본값)
	bIsDestructible = true;
}

void ABrawlBush::BeginPlay()
{
	Super::BeginPlay();

	// MeshComponent (은신 영역) 바인딩
	if (MeshComponent)
	{
		MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ABrawlBush::OnMeshOverlapBegin);
		MeshComponent->OnComponentEndOverlap.AddDynamic(this, &ABrawlBush::OnMeshOverlapEnd);

		// 초기 투명도 설정
		CurrentOpacity = NormalOpacity;
		TargetOpacity = NormalOpacity;
		
		// 초기 회전값 저장
		InitialRotation = MeshComponent->GetRelativeRotation();

		// 다이내믹 머티리얼 인스턴스 생성 (투명도 조절용)
		if (UMaterialInterface* BaseMat = MeshComponent->GetMaterial(0))
		{
			BushMID = MeshComponent->CreateDynamicMaterialInstance(0, BaseMat);
			if (BushMID)
			{
				BushMID->SetScalarParameterValue(OpacityParamName, CurrentOpacity);
			}
		}
	}

	// ProximitySphere (접근 영역) 바인딩
	if (ProximitySphere)
	{
		ProximitySphere->SetSphereRadius(ProximityRadius);
		ProximitySphere->OnComponentBeginOverlap.AddDynamic(this, &ABrawlBush::OnProximityOverlapBegin);
		ProximitySphere->OnComponentEndOverlap.AddDynamic(this, &ABrawlBush::OnProximityOverlapEnd);
	}
}

void ABrawlBush::OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	if (ABrawlCharacter* Character = Cast<ABrawlCharacter>(OtherActor))
	{
		// 1. 캐릭터에게 "수풀 진입" 알림 (은신 처리용)
		Character->SetInBush(true);
		
		// 2. 관리 목록에 추가
		CharactersInside.Add(Character);

		// 3. 현재 주변에 감지자가 있다면, 이 캐릭터를 감지자에게 드러내야 함
		UpdateVisibilityForHiddenCharacters();
	}
}

void ABrawlBush::OnMeshOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	if (ABrawlCharacter* Character = Cast<ABrawlCharacter>(OtherActor))
	{
		// 1. 캐릭터에게 "수풀 나감" 알림
		Character->SetInBush(false);

		// 2. 관리 목록에서 제거
		CharactersInside.Remove(Character);
		
		// 3. 나가면 어차피 은신 풀리므로 별도 처리 불필요 
		// (SetInBush(false)에서 처리됨)
	}
}

void ABrawlBush::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 투명도 보간
	if (!FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity))
	{
		CurrentOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, DeltaTime, FadingSpeed);
		
		if (BushMID)
		{
			BushMID->SetScalarParameterValue(OpacityParamName, CurrentOpacity);
		}
	}

	// 2. 수풀 흔들림 (Wiggle) 처리
	bool bIsAnyCharacterMoving = false;

	// 안에 있는 캐릭터 중 하나라도 움직이고 있는지 확인
	for (ABrawlCharacter* Char : CharactersInside)
	{
		if (Char && Char->GetVelocity().SizeSquared() > 100.0f) // 약간의 움직임이라도 있으면
		{
			bIsAnyCharacterMoving = true;
			break;
		}
	}

	if (MeshComponent)
	{
		FRotator TargetRotation = InitialRotation;

		if (bIsAnyCharacterMoving)
		{
			// 시간 기반 Sine 파동으로 흔들림 생성
			float Time = GetWorld()->GetTimeSeconds();
			float PitchOffset = FMath::Sin(Time * SwaySpeed) * SwayStrength;
			float RollOffset = FMath::Cos(Time * SwaySpeed * 0.8f) * SwayStrength; // 주기를 다르게 하여 불규칙성 추가

			TargetRotation += FRotator(PitchOffset, 0.0f, RollOffset);
		}

		// 현재 회전에서 목표 회전으로 부드럽게 보간 (RInterpTo)
		FRotator NewRotation = FMath::RInterpTo(MeshComponent->GetRelativeRotation(), TargetRotation, DeltaTime, 10.0f);
		MeshComponent->SetRelativeRotation(NewRotation);
	}
}

void ABrawlBush::OnProximityOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	if (ABrawlCharacter* Character = Cast<ABrawlCharacter>(OtherActor))
	{
		// 접근자 목록 추가
		CharactersNearby.Add(Character);

		// 1. 플레이어가 제어 중이라면 목표 투명도 변경
		// TODO: 플레이어와 같은 팀이 제어 중인지 여부를 검사해야 함
		if (Character->IsPlayerControlled())
		{
			TargetOpacity = TranslucentOpacity;
		}

		// 2. 숨어있는 캐릭터들에 대해 "감지됨" 처리
		UpdateVisibilityForHiddenCharacters();
	}
}

void ABrawlBush::OnProximityOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	if (ABrawlCharacter* Character = Cast<ABrawlCharacter>(OtherActor))
	{
		// 접근자 목록 제거
		CharactersNearby.Remove(Character);

		// 1. 로컬 플레이어라면 목표 투명도 복구
		if (Character->IsLocallyControlled())
		{
			TargetOpacity = NormalOpacity;
		}
		
		// 2. 이 캐릭터가 감지하고 있던 숨은 캐릭터들의 상태 업데이트
		UpdateVisibilityForHiddenCharacters();
	}
}

void ABrawlBush::UpdateVisibilityForHiddenCharacters()
{
	// 숨어있는 모든 캐릭터에 대해
	for (ABrawlCharacter* HiddenChar : CharactersInside)
	{
		if (!HiddenChar) continue;

		// 이 캐릭터를 감지할 수 있는 사람(접근자)이 있는지 확인
		bool bRevealed = false;
		
		// 접근자가 한 명이라도 있으면 (일단 지금은 팀 구분 없이)
		// 실제로는: "적군"이 접근했을 때만 Revealed 되어야 하고, "아군"은 항상 보여야 함.
		// 프로토타입 단계에서는 "누군가 접근하면 보임"으로 처리.
		if (CharactersNearby.Num() > 0)
		{
			bRevealed = true;
		}

		// 캐릭터에게 "누군가에 의해 감지되고 있음"을 알림
		HiddenChar->SetRevealed(bRevealed);
	}
}