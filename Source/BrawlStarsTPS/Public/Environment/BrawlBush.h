// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Environment/BrawlObstacle.h"
#include "BrawlBush.generated.h"

class UBoxComponent;
class USphereComponent;

/**
 * ABrawlBush
 * 
 * 캐릭터가 숨을 수 있는 수풀 클래스.
 * - 캐릭터가 들어가면 투명해짐 (로컬 플레이어 기준)
 * - 발사체 및 캐릭터 통과 가능
 * - AI 시야 차단 (BrawlCharacter의 SetInBush 호출)
 */
UCLASS()
class BRAWLSTARSTPS_API ABrawlBush : public ABrawlObstacle
{
	GENERATED_BODY()
	
public:
	ABrawlBush();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// Mesh 겹침 (실제 수풀 내부, 은신)
	UFUNCTION()
	void OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnMeshOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Sphere 겹침 (접근, 투명화 및 탐지)
	UFUNCTION()
	void OnProximityOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProximityOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 현재 이 수풀 안에 숨어있는 캐릭터 목록 */
	void UpdateVisibilityForHiddenCharacters();

protected:
	// 접근 감지용 스피어 (투명화 및 은신 감지)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bush")
	TObjectPtr<USphereComponent> ProximitySphere;

	// 접근 감지 반경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bush|Setting")
	float ProximityRadius = 200.0f;

	// 투명도 조절을 위한 다이내믹 머티리얼 인스턴스
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BushMID;

	// 투명도 파라미터 이름 (머티리얼에서 설정 필요)
	UPROPERTY(EditAnywhere, Category = "Bush|Visuals")
	FName OpacityParamName = FName("Opacity");

	// 평소 불투명도
	UPROPERTY(EditAnywhere, Category = "Bush|Visuals")
	float NormalOpacity = 1.0f;

	// 플레이어 진입 시 불투명도
	UPROPERTY(EditAnywhere, Category = "Bush|Visuals")
	float TranslucentOpacity = 0.05f;

	// 투명도 변경 속도
	UPROPERTY(EditAnywhere, Category = "Bush|Visuals")
	float FadingSpeed = 5.0f;

	// 현재 불투명도
	float CurrentOpacity = 1.0f;

	// 목표 불투명도
	float TargetOpacity = 1.0f;
	
	// 현재 수풀 메시(은신 영역)에 들어와 있는 캐릭터들
	UPROPERTY()
	TSet<TObjectPtr<class ABrawlCharacter>> CharactersInside;

	// 현재 수풀 근처(감지 영역)에 있는 캐릭터들
	UPROPERTY()
	TSet<TObjectPtr<class ABrawlCharacter>> CharactersNearby;

protected:
	// 수풀 흔들림 강도 (각도)
	UPROPERTY(EditAnywhere, Category = "Bush|Animation")
	float SwayStrength = 5.0f;

	// 수풀 흔들림 속도
	UPROPERTY(EditAnywhere, Category = "Bush|Animation")
	float SwaySpeed = 15.0f;

	// 원래 회전값 저장
	FRotator InitialRotation;
};
