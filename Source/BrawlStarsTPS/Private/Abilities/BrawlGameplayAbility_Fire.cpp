// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BrawlGameplayAbility_Fire.h"
#include "AbilitySystemComponent.h"
#include "BrawlCharacter.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/KismetMathLibrary.h"

UBrawlGameplayAbility_Fire::UBrawlGameplayAbility_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UBrawlGameplayAbility_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("BrawlGameplayAbility_Fire::ActivateAbility Called!"));

	// 1. 코스트 및 쿨다운 확인 및 지불
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("BrawlGameplayAbility_Fire::ActivateAbility Failed CommitAbility"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. Gameplay Event 대기
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

void UBrawlGameplayAbility_Fire::SpawnProjectile()
{
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnProjectile Failed: ProjectileClass is NULL"));
		return;
	}

	ACharacter* Character = CastChecked<ACharacter>(GetAvatarActorFromActorInfo());

	// 서버 권한 확인 (클라이언트 예측을 원하면 로직 추가 필요, 여기선 서버 권한만 체크)
	if (!Character->HasAuthority()) return;

	// 1. 발사 시작점 (Muzzle)
	FVector MuzzleLocation = Character->GetActorLocation();
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		if (Mesh->DoesSocketExist(MuzzleSocketName))
		{
			MuzzleLocation = Mesh->GetSocketLocation(MuzzleSocketName);
		}
	}

	// 2. 목표 지점 (Camera Aim) 계산
	FVector TargetLocation = MuzzleLocation + (Character->GetActorForwardVector() * 1000.0f); // 기본값

	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		FVector CameraLoc;
		FRotator CameraRot;
		PC->GetPlayerViewPoint(CameraLoc, CameraRot);

		// 카메라 앞쪽으로 레이캐스트
		FVector TraceStart = CameraLoc;
		FVector TraceEnd = CameraLoc + (CameraRot.Vector() * 5000.0f); // 50미터

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character); // 자신은 무시

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			TargetLocation = HitResult.ImpactPoint;
		}
		else
		{
			TargetLocation = TraceEnd;
		}
	}

	// 3. 발사 방향 회전 (Muzzle -> Target)
	FRotator ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, TargetLocation);

	// 4. 발사체 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	
	GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, ProjectileRotation, SpawnParams);
	
	// UE_LOG(LogTemp, Log, TEXT("Projectile Spawned at %s towards %s"), *MuzzleLocation.ToString(), *TargetLocation.ToString());
}

void UBrawlGameplayAbility_Fire::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
