// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlBush.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h" // 추가
#include "BrawlCharacter.h"

ABrawlBush::ABrawlBush()
{
	PrimaryActorTick.bCanEverTick = true;

	// 수풀 메시는 시각적 표현 및 장애물 역할
	if (MeshComponent)
	{
		// AI 시야 차단을 위해 Visibility 채널을 Block으로 설정
		MeshComponent->SetCollisionProfileName(FName("Custom"));
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComponent->SetCollisionObjectType(ECC_WorldStatic);

		// 기본적으로 모두 Overlap (기존 OverlapAll 동작 유지)
		MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
		
		// 1. AI 시야(Visibility)는 무시 -> 물리적으로는 투시 가능 (로직으로 처리)
		MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

		// 2. 카메라(Camera)는 무시 -> 카메라가 수풀 때문에 당겨지는 것 방지
		MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		
		// 3. 캐릭터(Pawn)는 무시 -> 은신 판정은 HidingVolume이 담당
		MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); 

		MeshComponent->SetGenerateOverlapEvents(false); // 메시 자체 오버랩 이벤트 끔
	}

	// 은신 판정용 내부 볼륨 (메시보다 작게 설정하여 중심 진입 시 은신)
	HidingVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("HidingVolume"));
	HidingVolume->SetupAttachment(RootComponent);
	HidingVolume->SetBoxExtent(FVector(35.0f, 35.0f, 50.0f)); // 기본 크기, 메시 크기에 맞춰 조정 필요
	HidingVolume->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.0f)); // 시각적 크기보다 작게 설정 (중심 판정 유도)
	
	HidingVolume->SetCollisionProfileName(FName("Custom"));
	HidingVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HidingVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	HidingVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HidingVolume->SetGenerateOverlapEvents(true);

	// 접근 감지용 스피어 생성
	ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
	ProximitySphere->SetupAttachment(RootComponent);
	ProximitySphere->SetSphereRadius(ProximityRadius);
	
	// 기본 프로필 대신 커스텀 설정 사용
	ProximitySphere->SetCollisionProfileName(FName("Custom")); 
	ProximitySphere->SetCollisionResponseToAllChannels(ECR_Ignore); // 모든 채널 무시
	ProximitySphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 캐릭터(Pawn)만 감지
	
	ProximitySphere->SetGenerateOverlapEvents(true);

	// 수풀은 파괴 가능함 (기본값)
	bIsDestructible = true;
}

void ABrawlBush::BeginPlay()
{
	Super::BeginPlay();

	// HidingVolume (은신 영역) 바인딩
	if (HidingVolume)
	{
		HidingVolume->OnComponentBeginOverlap.AddDynamic(this, &ABrawlBush::OnHidingOverlapBegin);
		HidingVolume->OnComponentEndOverlap.AddDynamic(this, &ABrawlBush::OnHidingOverlapEnd);
	}

	if (MeshComponent)
	{
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

void ABrawlBush::OnHidingOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

void ABrawlBush::OnHidingOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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
		
		for (ABrawlCharacter* NearbyChar : CharactersNearby)
		{
			if (!NearbyChar) continue;
			
			// 자기 자신은 감지자가 될 수 없음
			if (HiddenChar == NearbyChar) continue;
			
			// 같은 팀은 감지하지 않음 (팀 ID 비교)
			// 255(NoTeam)는 모두와 적대적이라고 가정 (FFA 등)
			uint8 HiddenTeam = HiddenChar->GetGenericTeamId().GetId();
			uint8 NearbyTeam = NearbyChar->GetGenericTeamId().GetId();
			
			// 서로 다른 팀이거나, 둘 다 팀이 없는 경우(FFA) 적대 관계 -> 발각됨
			if (HiddenTeam != NearbyTeam || (HiddenTeam == 255 && NearbyTeam == 255))
			{
				bRevealed = true;
				break;
			}
		}

		// 캐릭터에게 "누군가에 의해 감지되고 있음"을 알림
		HiddenChar->SetRevealed(bRevealed);
	}
}