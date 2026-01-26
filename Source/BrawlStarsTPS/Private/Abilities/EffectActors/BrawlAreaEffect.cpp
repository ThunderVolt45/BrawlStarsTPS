// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/EffectActors/BrawlAreaEffect.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "TimerManager.h"

ABrawlAreaEffect::ABrawlAreaEffect()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 서버에서 생성되어 클라이언트에 보여야 함

	// 1. Root Component (Scene)
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// 2. Area Box
	Area = CreateDefaultSubobject<USphereComponent>(TEXT("AreaBox"));
	Area->SetupAttachment(RootComponent);
	Area->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Area->SetCollisionObjectType(ECC_WorldDynamic);
	Area->SetCollisionResponseToAllChannels(ECR_Ignore);
	Area->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 폰만 감지
	
	// 박스 크기 기본값 (반지름 200 정도 느낌)
	Area->SetSphereRadius(100.0f);

	// 3. Decal
	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(RootComponent);
	DecalComponent->DecalSize = FVector(100.f, 200.f, 200.f); // 투영 깊이, Y, Z
	DecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); // 바닥을 향하도록
}

void ABrawlAreaEffect::BeginPlay()
{
	Super::BeginPlay();

	// 오버랩 이벤트 바인딩
	if (Area)
	{
		Area->OnComponentBeginOverlap.AddDynamic(this, &ABrawlAreaEffect::OnOverlapBegin);
		Area->OnComponentEndOverlap.AddDynamic(this, &ABrawlAreaEffect::OnOverlapEnd);
	}

	// 수명 설정 (자동 파괴)
	SetLifeSpan(Duration);

	// 주기적 효과 타이머 시작
	if (Period > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(PeriodTimerHandle, this, &ABrawlAreaEffect::ApplyPeriodicEffect, Period, true);
	}
}

void ABrawlAreaEffect::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	// 중복 추가 방지
	if (!OverlappingActors.Contains(OtherActor))
	{
		OverlappingActors.Add(OtherActor);
	}
}

void ABrawlAreaEffect::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	OverlappingActors.Remove(OtherActor);
}

void ABrawlAreaEffect::ApplyPeriodicEffect()
{
	// 자식 클래스에서 구현
	// 예: OverlappingActors를 순회하며 데미지 주기
	
	// 유효하지 않은 액터 정리
	for (int32 i = OverlappingActors.Num() - 1; i >= 0; i--)
	{
		if (!IsValid(OverlappingActors[i]))
		{
			OverlappingActors.RemoveAt(i);
		}
	}
}
