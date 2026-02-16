// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Fire.h"
#include "AbilitySystemComponent.h"
#include "BrawlAttributeSet.h"
#include "BrawlCharacter.h"
#include "BrawlProjectile.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "BrawlStarsTPSPlayerController.h"
#include "BrawlPoolSubsystem.h"

UBrawlGameplayAbility_Fire::UBrawlGameplayAbility_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 발사 중임을 나타내는 태그 (Reload 등 다른 동작 차단용)
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Fire")));
}

void UBrawlGameplayAbility_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// UE_LOG(LogTemp, Warning, TEXT("BrawlGameplayAbility_Fire::ActivateAbility Called!"));

	// 1. 코스트 및 쿨다운 확인 및 지불
	// CommitAbility는 Cost GE가 없으면 true를 반환하지만 ApplyCost를 부르지 않음.
	// 따라서 CommitAbility 호출 후, 강제로 ApplyCost를 호출하여 C++ 변수 기반 차감을 수행.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// UE_LOG(LogTemp, Warning, TEXT("BrawlGameplayAbility_Fire::ActivateAbility Failed CommitAbility"));
		
		// 실패 원인이 코스트(탄약)인지 확인하기 위해 CheckCost를 다시 호출해봄
		FGameplayTagContainer FailureTags;
		if (!CheckCost(Handle, ActorInfo, &FailureTags))
		{
			// 코스트 부족으로 실패 (가젯은 CheckCost가 true이므로 여기 안 들어옴)
			if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
			{
				if (ABrawlStarsTPSPlayerController* PC = Cast<ABrawlStarsTPSPlayerController>(Character->GetController()))
				{
					// 로컬 컨트롤러라면 UI 애니메이션 재생
					// (서버에서 호출되더라도 Client RPC이므로 클라이언트로 전송됨)
					PC->PlayNoAmmoAnimation();
				}
			}
		}

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. Gameplay Event 대기 (Event.Weapon.Fire)
	// 몽타주에서 노티파이로 이벤트를 보내면 OnFireEventReceived가 호출됨
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireEventTag);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnFireEventReceived);
		WaitEventTask->ReadyForActivation();
	}

	// 3. 몽타주 재생
	// PlayMontageAndWait를 쓰면 몽타주 종료 시점까지 어빌리티를 유지할 수 있음
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FireMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnMontageEnded);
		MontageTask->OnBlendOut.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UBrawlGameplayAbility_Fire::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주 재생 실패 시 즉시 발사 시도 (안전장치)
		SpawnProjectile();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UBrawlGameplayAbility_Fire::OnFireEventReceived(FGameplayEventData Payload)
{
	// 이벤트 수신 시 발사체 스폰
	SpawnProjectile();
}

void UBrawlGameplayAbility_Fire::GetProjectilePrewarmData(TMap<TSubclassOf<AActor>, int32>& OutData) const
{
	if (ProjectileClass) OutData.FindOrAdd(ProjectileClass) += PrewarmCount;
	if (ProjectileClass_Hyper) OutData.FindOrAdd(ProjectileClass_Hyper) += PrewarmCount;
}

TSubclassOf<AActor> UBrawlGameplayAbility_Fire::GetProjectileClassToSpawn() const
{
	// 기본 발사체
	TSubclassOf<AActor> ClassToSpawn = ProjectileClass;
	
	// 만약 하이퍼차지 상태고 하이퍼차지용 발사체가 있다면 발사체 교체
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		static FGameplayTag HyperStateTag = FGameplayTag::RequestGameplayTag(FName("State.Hypercharged"));
		if (ASC->HasMatchingGameplayTag(HyperStateTag))
		{
			if (ProjectileClass_Hyper)
			{
				ClassToSpawn = ProjectileClass_Hyper;
			}
		}
	}
	
	return ClassToSpawn;
}

FVector UBrawlGameplayAbility_Fire::GetMuzzleLocation(FName SocketName, FName ParentSocketName) const
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return FVector::ZeroVector;

	if (SocketName.IsNone()) return Character->GetActorLocation();

	// 1. 메인 메쉬 확인 (ParentSocketName이 없거나 일치할 때만)
	if (ParentSocketName.IsNone())
	{
		if (USkeletalMeshComponent* MainMesh = Character->GetMesh())
		{
			if (MainMesh->DoesSocketExist(SocketName))
			{
				return MainMesh->GetSocketLocation(SocketName);
			}
		}
	}

	// 2. 자식 컴포넌트(Mesh Component) 확인
	TArray<UMeshComponent*> MeshComponents;
	Character->GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComp : MeshComponents)
	{
		if (!ParentSocketName.IsNone())
		{
			if (MeshComp->GetAttachSocketName() != ParentSocketName)
			{
				continue;
			}
		}

		if (MeshComp->DoesSocketExist(SocketName))
		{
			return MeshComp->GetSocketLocation(SocketName);
		}
	}

	// 못 찾았으면 기본 위치 (경고는 호출 측에서 판단하거나 여기서 띄움)
	UE_LOG(LogTemp, Warning, TEXT("GetMuzzleLocation: Socket [%s] (Parent: %s) not found!"), *SocketName.ToString(), *ParentSocketName.ToString());
	return Character->GetActorLocation();
}

FRotator UBrawlGameplayAbility_Fire::GetAimRotation(FVector StartLocation) const
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return FRotator::ZeroRotator;
	
	// 기본 회전 값은 컨트롤러의 회전 값으로 한다
	FRotator BaseRotation = Character->GetControlRotation();
	BaseRotation.Pitch += 3.0f;

	// ABrawlCharacter로 캐스팅
	ABrawlCharacter* BrawlCharacter = Cast<ABrawlCharacter>(Character);
	
	// 캐스팅에 실패했거나 AI 브롤러인 경우 기본 값 반환
	if (!BrawlCharacter && !BrawlCharacter->IsPlayerControlled())
	{
		return BaseRotation;
	}

	// 플레이어의 경우, 카메라 레이캐스트를 통한 정밀 보정 및 자동 조준 대응
	if (ABrawlStarsTPSPlayerController* PC = Cast<ABrawlStarsTPSPlayerController>(Character->GetController()))
	{
		// 카메라 정보 가져오기
		FVector CameraLoc;
		FRotator CameraRot;
		PC->GetPlayerViewPoint(CameraLoc, CameraRot);
		FVector CameraForward = CameraRot.Vector();
		FVector ControllerForward = PC->GetControlRotation().Vector();

		// 1. 자동 조준(Auto-Aim) 예측 지점이 있다면 사용
		FVector PredictedLoc = PC->GetPredictedAimLocation();
		if (!PredictedLoc.IsZero())
		{
			// 총구에서 예측 지점까지의 기본 회전값
			FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(StartLocation, PredictedLoc);
			
			// [Fix] 각도 제한 로직: 컨트롤러 정면 방향과 조준 방향의 각도 차이를 계산
			float AngleDiff = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ControllerForward, TargetRot.Vector())));
			
			// 최대 허용 각도를 벗어나면 카메라 정면 방향으로 보정
			const float MaxAutoAimAngle = 5.0f;
			if (AngleDiff > MaxAutoAimAngle)
			{
				FQuat ControllerQuat = ControllerForward.ToOrientationQuat();
				FQuat TargetQuat = TargetRot.Quaternion();
				float Alpha = MaxAutoAimAngle / AngleDiff;
				FQuat ClampedQuat = FQuat::Slerp(ControllerQuat, TargetQuat, Alpha);
				return ClampedQuat.Rotator();
			}

			return TargetRot;
		}

		// 2. 수동 조준 (목표 없음): 화면 중앙(카메라 전방)을 향해 발사
		FVector TargetLocation = CameraLoc + (CameraForward * AimMaxRange);
		
		// Muzzle에서 바라본 TargetLocation의 각도
		return UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);
	}
	
	return BaseRotation;
}

FGameplayEffectSpecHandle UBrawlGameplayAbility_Fire::MakeDamageSpecHandle(float DamageScale) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !DamageEffectClass) return FGameplayEffectSpecHandle();

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);
	if (SpecHandle.IsValid())
	{
		// 데미지 양 설정 (GetDamageAttribute에서 가져옴)
		bool bFound = false;
		float DamageValue = ASC->GetGameplayAttributeValue(GetDamageAttribute(), bFound);
				
		// 못 찾았으면 기본값(DamageAmount) 사용
		if (!bFound) DamageValue = DamageAmount;

		// 데미지 적용 (Scale 적용)
		float FinalDamage = FMath::Abs(DamageValue) * DamageScale;

		static FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, FinalDamage);
	}

	return SpecHandle;
}

void UBrawlGameplayAbility_Fire::SpawnProjectile(FName AttachParentSocketName)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;
	
	// 1. 발사체 클래스
	TSubclassOf<AActor> ClassToSpawn = GetProjectileClassToSpawn();
	if (!ClassToSpawn) 
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnProjectile Failed: ProjectileClass is NULL"));
		return;
	}

	// 2. 발사 시작점
	FVector MuzzleLocation = GetMuzzleLocation(MuzzleSocketName, AttachParentSocketName);

	// 3. 발사 방향
	FRotator BaseRotation = GetAimRotation(MuzzleLocation);
	FVector BaseDirection = BaseRotation.Vector();

	// 4. 발사체 스폰 (Multi-Projectile Logic)
	int32 RealCount = FMath::Max(1, ProjectileCount);
	float ConeHalfAngleRad = FMath::DegreesToRadians(SpreadAngle);
	FRandomStream WeaponRandomStream(FMath::Rand());

	// 데미지 스펙 미리 생성
	FGameplayEffectSpecHandle SpecHandle = MakeDamageSpecHandle(DamagePerPelletScale);

	UBrawlPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UBrawlPoolSubsystem>();

	for (int32 i = 0; i < RealCount; i++)
	{
		// 원뿔 내 무작위 방향 벡터 생성
		FVector LaunchDir = ConeHalfAngleRad > UE_SMALL_NUMBER ? 
			WeaponRandomStream.VRandCone(BaseDirection, ConeHalfAngleRad) : BaseDirection;
		
		FRotator FinalRotation = LaunchDir.Rotation();

		FTransform SpawnTransform(FinalRotation, MuzzleLocation);
		
		AActor* SpawnedActor = PoolSubsystem ? 
			PoolSubsystem->GetFromPool(ClassToSpawn, SpawnTransform, Character, Character) :
			GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnTransform);
		
		if (ABrawlProjectile* Projectile = Cast<ABrawlProjectile>(SpawnedActor))
		{
			// 미리 만든 Spec 주입
			if (SpecHandle.IsValid())
			{
				Projectile->InitializeProjectile(SpecHandle);
			}
		}
	}

	// SFX/VFX 재생 (블루프린트에서 설정한 AbilityGameplayCueTag 사용)
	PlayGameplayCue(MuzzleLocation, BaseRotation.Vector());
}

FGameplayAttribute UBrawlGameplayAbility_Fire::GetDamageAttribute() const
{
	return UBrawlAttributeSet::GetAttackDamageAttribute();
}

void UBrawlGameplayAbility_Fire::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
