// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/BrawlTieBreaker.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BrawlCharacter.h"
#include "GameMode/BrawlGameMode_Bounty.h"

ABrawlTieBreaker::ABrawlTieBreaker()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(100.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ABrawlTieBreaker::OnOverlapBegin);
}

void ABrawlTieBreaker::BeginPlay()
{
	Super::BeginPlay();
}

void ABrawlTieBreaker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(FRotator(0, 90.0f * DeltaTime, 0));
}

void ABrawlTieBreaker::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if (ABrawlCharacter* Character = Cast<ABrawlCharacter>(OtherActor))
	{
		if (ABrawlGameMode_Bounty* GM = GetWorld()->GetAuthGameMode<ABrawlGameMode_Bounty>())
		{
			GM->OnTieBreakerPickedUp(Character);
			Destroy();
		}
	}
}
