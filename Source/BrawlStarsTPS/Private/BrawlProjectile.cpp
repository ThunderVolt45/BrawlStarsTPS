// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BrawlCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Environment/BrawlDestructibleInterface.h"
#include "Environment/BrawlBush.h" 
#include "GenericTeamAgentInterface.h"
#include "BrawlPoolSubsystem.h"

ABrawlProjectile::ABrawlProjectile()
{
	PrimaryActorTick.bCanEverTick = true; // 관통 및 레이캐스트 로직을 위해 Tick 활성화
	bReplicates = true;

	bIsActive = true; // 기본적으로 true (SpawnActor로 생성될 때 대응)

	// 1. 충돌체 설정
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	
	// 1-1. 충돌 범위 설정
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_Destructible, ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCanEverAffectNavigation(false); // NavMesh에 영향 주지 않음
	
	// 1-2. 충돌 이벤트 바인딩
	SphereComponent->OnComponentHit.AddDynamic(this, &ABrawlProjectile::OnHit);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABrawlProjectile::OnBeginOverlap);

	// 2. 외형 설정
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(SphereComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetCanEverAffectNavigation(false); // NavMesh에 영향 주지 않음

	// 3. 이동 컴포넌트 설정
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ABrawlProjectile::InitializeProjectile(const FGameplayEffectSpecHandle& InDamageSpecHandle)
{
	DamageSpecHandle = InDamageSpecHandle;
	
	if (!DamageSpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Projectile [%s] Initialized with INVALID Damage Spec! Make sure DamageEffectClass is assigned in the calling Ability."), *GetName());
		
		// 개발 중 에디터에서 즉시 문제를 인지할 수 있도록 ensure 추가
		ensureAlwaysMsgf(false, TEXT("Projectile [%s] has no valid Damage Spec. Damage will not be applied."), *GetName());
	}
}

void ABrawlProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 처음 생성될 때는 OnActivate를 수동으로 부르지 않으므로 여기서 초기화
	// (단, 풀 서브시스템을 통해 생성되는 경우 GetFromPool에서 호출됨)
	if (bIsActive)
	{
		OnActivate();
	}
}

void ABrawlProjectile::OnActivate()
{
	bIsActive = true;
	SetActorHiddenInGame(false);
	
	// 1. 충돌 설정 초기화
	if (SphereComponent)
	{
		// 생성 즉시 충돌하여 사라지는 것을 방지하기 위해 일시적으로 비활성화
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
		SphereComponent->SetGenerateOverlapEvents(true);

		// 이전에 설정된 무시 목록 초기화
		SphereComponent->ClearMoveIgnoreActors();

		if (bCanPierce)
		{
			// 관통형: 모든 채널을 Overlap으로 설정하여 멈추지 않게 함
			SphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
		}
		else
		{
			// 일반형: 기본 응답으로 복구 (Block) 및 발사체끼리만 무시
			SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);
			SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
			// 가시성 채널 등 불필요한 채널 무시
			SphereComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		}

		// 카메라 충돌은 항상 무시 (카메라 튐 방지)
		SphereComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		// 발사자/주인 무시 다시 설정
		if (AActor* MyInstigator = GetInstigator()) SphereComponent->IgnoreActorWhenMoving(MyInstigator, true);
		if (AActor* MyOwner = GetOwner()) SphereComponent->IgnoreActorWhenMoving(MyOwner, true);

		// 아주 짧은 시간 뒤에 충돌 활성화
		GetWorldTimerManager().SetTimer(CollisionDelayTimerHandle, this, &ABrawlProjectile::EnableCollisionDelayed, 0.01f, false);
	}

	// 2. 이동 초기화
	if (ProjectileMovement)
	{
		// 기존 속도 및 상태 초기화
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileSpeed;
		
		// 컴포넌트 활성화 및 업데이트 강제
		ProjectileMovement->Activate(true); 
		ProjectileMovement->SetUpdatedComponent(SphereComponent);
	}

	// 3. 내부 상태 초기화
	HitActors.Empty();
	PreviousLocation = GetActorLocation();
	
	// 관통형 발사체는 직접 충돌을 검사해야하므로 Tick 활성화
	SetActorTickEnabled(bCanPierce);

	// 수명 타이머 재설정
	GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &ABrawlProjectile::OnLifeTimeExpired, LifeTime, false);
}

void ABrawlProjectile::OnDeactivate()
{
	bIsActive = false;
	SetActorHiddenInGame(true);
	
	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorTickEnabled(false);

	// 타이머 해제
	GetWorldTimerManager().ClearTimer(LifeTimerHandle);
	GetWorldTimerManager().ClearTimer(CollisionDelayTimerHandle);

	// 상태 및 데이터 초기화
	DamageSpecHandle = FGameplayEffectSpecHandle();
	HitActors.Empty();

	// 이동 정지
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
}

void ABrawlProjectile::EnableCollisionDelayed()
{
	if (SphereComponent && bIsActive)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ABrawlProjectile::Deactivate()
{
	if (UBrawlPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UBrawlPoolSubsystem>())
	{
		PoolSubsystem->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}

void ABrawlProjectile::OnLifeTimeExpired()
{
	Deactivate();
}

void ABrawlProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 관통형 발사체가 아니라면 중단
	if (!bCanPierce || !SphereComponent)
	{
		return;
	}

	// 관통형 발사체인 경우, 이동 경로에 대한 스윕(Sweep) 검사 수행 (터널링 방지)
	FVector CurrentLocation = GetActorLocation();

	// 움직임이 거의 없으면 스킵
	if (FVector::DistSquared(PreviousLocation, CurrentLocation) < 1.0f)
	{
		return;
	}

	// 레이캐스트 준비
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	
	// 발사자(Instigator)는 무시
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
	if (GetInstigator()) QueryParams.AddIgnoredActor(GetInstigator());

	// Pawn과 WorldStatic, WorldDynamic에 대해 검사
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	// 구체 스윕 (경로상 모든 물체 감지)
	bool bHit = GetWorld()->SweepMultiByObjectType(
		HitResults,
		PreviousLocation,
		CurrentLocation,
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(SphereComponent->GetScaledSphereRadius()),
		QueryParams
	);

	if (bHit)
	{
		for (const FHitResult& Result : HitResults)
		{
			AActor* HitActor = Result.GetActor();
			UPrimitiveComponent* HitComp = Result.GetComponent();

			// 수풀의 감지용 스피어는 무시
			if (HitActor && HitActor->IsA(ABrawlBush::StaticClass()) && HitComp && HitComp->GetName().Contains(TEXT("ProximitySphere")))
			{
				continue;
			}

			if (HitActor && !HitActors.Contains(HitActor))
			{
				// 발사체끼리는 무시
				if (HitActor->IsA(StaticClass())) continue;
				
				// 처리 등록
				HitActors.Add(HitActor);
				
				// 충돌 처리
				// UE_LOG(LogTemp, Log, TEXT("Projectile Sweep Hit: %s"), *HitActor->GetName());
				
				PlayHitEffects(Result.ImpactPoint, Result.Normal, HitActor);
				ProcessHit(HitActor, Result.ImpactPoint);
			}
		}
	}

	PreviousLocation = CurrentLocation;
}


void ABrawlProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 발사자(Instigator)는 무시
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator() 
		|| OtherActor == this || OtherActor->IsA(StaticClass())) 
		return;

	// 수풀의 감지용 스피어는 무시
	if (OtherActor->IsA(ABrawlBush::StaticClass()) && OtherComp && OtherComp->GetName().Contains(TEXT("ProximitySphere")))
	{
		return;
	}

	// 이미 처리된 액터면 무시 (관통 시 중복 방지)
	if (HitActors.Contains(OtherActor)) return;
	HitActors.Add(OtherActor);
	
	// 충돌 처리
	// UE_LOG(LogTemp, Log, TEXT("Projectile HIT Block: %s"), *OtherActor->GetName());
	
	PlayHitEffects(Hit.ImpactPoint, Hit.Normal, OtherActor);
	ProcessHit(OtherActor, Hit.ImpactPoint);
}

void ABrawlProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 발사자(Instigator)는 무시
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()
		|| OtherActor == this || OtherActor->IsA(StaticClass()))
		return;

	// 수풀의 감지용 스피어는 무시
	if (OtherActor->IsA(ABrawlBush::StaticClass()) && OtherComp && OtherComp->GetName().Contains(TEXT("ProximitySphere")))
	{
		return;
	}

	// 이미 처리된 액터면 무시
	if (HitActors.Contains(OtherActor)) return;
	HitActors.Add(OtherActor);
	
	// 충돌 처리
	// UE_LOG(LogTemp, Log, TEXT("Projectile HIT Overlap: %s"), *OtherActor->GetName());
	
	// Overlap의 경우 Normal 정보를 정확히 알기 어려우므로 진행 방향의 반대를 사용하거나 SweepResult 활용
	FVector Normal = bFromSweep ? FVector(SweepResult.Normal) : (GetActorRotation().Vector() * -1.0f);
	FVector Location = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();
	
	PlayHitEffects(Location, Normal, OtherActor);
	ProcessHit(OtherActor, Location);
}

void ABrawlProjectile::PlayHitEffects(const FVector& HitLocation, const FVector& HitNormal, AActor* HitActor)
{
	// 명중 효과 재생 (Gameplay Cue)
	if (HitGameplayCueTag.IsValid())
	{
		AActor* SourceActor = GetInstigator();
		if (!SourceActor) SourceActor = GetOwner();
		
		if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor))
		{
			FGameplayCueParameters Params;
			Params.Location = HitLocation;
			Params.Normal = HitNormal;
			Params.Instigator = SourceActor;
			
			// TargetActor가 null이면 ensure가 발생할 수 있으므로, 
			// EffectCauser를 통해 명시적으로 HitActor 혹은 SourceActor를 넘겨준다
			Params.EffectCauser = HitActor ? HitActor : SourceActor;

			SourceASC->ExecuteGameplayCue(HitGameplayCueTag, Params);
		}
	}
}

void ABrawlProjectile::ProcessHit(AActor* OtherActor, const FVector& HitLocation)
{
	if (!OtherActor) return;
	
	// 디버그 구체 그리기
	if (GetWorld())
	{
		DrawDebugSphere(GetWorld(), HitLocation, 10.0f, 12, FColor::Red, false, 2.0f);
	}

	// 팀 관계 확인
	bool bIsHostile = true;
	if (ABrawlCharacter* MyInstigator = Cast<ABrawlCharacter>(GetInstigator()))
	{
		// IsAlly를 사용하여 아군(생성자/주인 관계 포함) 여부 판별
		if (MyInstigator->IsAlly(OtherActor))
		{
			bIsHostile = false;
		}
	}
	else if (IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(OtherActor))
	{
		// Instigator가 ABrawlCharacter가 아닌 경우 기존 로직 수행
		if (IGenericTeamAgentInterface* MyInstigatorAgent = Cast<IGenericTeamAgentInterface>(GetInstigator()))
		{
			uint8 MyTeamID = MyInstigatorAgent->GetGenericTeamId().GetId();
			uint8 TargetTeamID = TargetTeamAgent->GetGenericTeamId().GetId();

			if (MyTeamID != 255 && TargetTeamID != 255 && MyTeamID == TargetTeamID)
			{
				bIsHostile = false;
			}
		}
	}

	// 데미지를 입힐 수 있는 액터이고 적대적인 경우에만 데미지 적용
	if (bIsHostile)
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			if (DamageSpecHandle.IsValid())
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			}
		}
	}
	
	// 맞은 액터가 파괴 가능한 장애물인지, "단단한" 장애물인지 확인
	bool bIsDestructibleObstacle = false;
	bool bIsHardObstacle = false;
	if (bDestroyObstacles)
	{
		if (IBrawlDestructibleInterface* Destructible = Cast<IBrawlDestructibleInterface>(OtherActor))
		{
			bIsDestructibleObstacle = Destructible->IsDestructible();
			bIsHardObstacle = Destructible->IsHardObstacle();
		}
	}
	
	// 장애물 파괴 로직
	if (bIsDestructibleObstacle && bDestroyObstacles)
	{
		DestroyObstacle(OtherActor);
		
		// 장애물 관통 능력이 없다면 파괴
		if (!bCanPierceHardObstacle && bIsHardObstacle)
		{
			Deactivate();
			return;
		}
		
		// 파괴 가능한 벽을 뚫을 수 있는 발사체가 파괴 가능한 벽과 충돌했다면 벽을 무시하고 계속 진행
		if (SphereComponent)
		{
			SphereComponent->IgnoreActorWhenMoving(OtherActor, true);
		}

		// 충돌로 잃은 속도 복구
		if (ProjectileMovement)
		{
			ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * ProjectileSpeed;
		}
	}
	
	// 관통 능력이 없다면 파괴
	if (!bCanPierce)
	{
		Deactivate();
	}
}

void ABrawlProjectile::DestroyObstacle(AActor* OtherActor)
{
	// 인터페이스 캐스팅 시도
	if (IBrawlDestructibleInterface* Destructible = Cast<IBrawlDestructibleInterface>(OtherActor))
	{
		if (Destructible->IsDestructible())
		{
			Destructible->OnDestruction(GetInstigator());
		}
	}
}
