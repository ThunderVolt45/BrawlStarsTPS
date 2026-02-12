// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlKillLogEntry.generated.h"

class UImage;
class UTextBlock;
/**
 * 브롤러 처치 로그가 표시될 위젯
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlKillLogEntry : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	// 처치한 브롤러의 아이콘이 표시될 이미지
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> KillerImage;
	
	// 처치한 브롤러의 이름이 표시될 텍스트
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> KillerNameText;
	
	// 처치당한 브롤러의 아이콘이 표시될 이미지
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> VictimImage;
	
	// 처치당한 브롤러의 이름이 표시될 텍스트
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> VictimNameText;
	
	// 위젯의 배경 이미지
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> BackgroundImage;
	
	// 아군 -> 적, 적 -> 적 처치 배경 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	FColor AllyKillBackgroundColor = FColor::Blue;
	
	// 적 -> 아군 처치 배경 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	FColor EnemyKillBackgroundColor = FColor::Red;
	
	// 표시 지속 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Animation")
	float DisplayDuration = 4.0f;

	// 애니메이션 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Animation")
	float AnimationSpeed = 10.0f;

	// 등장/퇴장 시 이동할 오프셋 (X, Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Animation")
	FVector2D SlideOffset = FVector2D(500.0f, 0.0f);

	// --- SFX ---
	// 아군이 적을 처치했을 때 재생할 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Sound")
	TObjectPtr<USoundBase> GoodKillSFX;

	// 적이 아군을 처치했을 때 재생할 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Sound")
	TObjectPtr<USoundBase> BadKillSFX;

public:
	// 킬 정보 설정 (액터 전달)
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetKillInfo(AActor* Killer, AActor* Victim);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 블루프린트에서 UI 업데이트 (텍스트 설정, 색상 변경 등)
	// Killer/Victim이 ABrawlCharacter일 수 있으므로 Cast해서 정보 사용
	UFUNCTION(BlueprintImplementableEvent, Category = "Brawl|UI")
	void OnKillInfoSet(AActor* Killer, AActor* Victim, bool bIsKillerMyself, bool bIsVictimMyself);

private:
	// 퇴장 애니메이션 시작
	void StartOutro();

	// 애니메이션 상태
	enum class EAnimationState
	{
		Intro, // 등장 중
		Idle,  // 대기 중
		Outro  // 사라지는 중
	};

	EAnimationState AnimState = EAnimationState::Intro;
	FTimerHandle OutroTimerHandle;
};
