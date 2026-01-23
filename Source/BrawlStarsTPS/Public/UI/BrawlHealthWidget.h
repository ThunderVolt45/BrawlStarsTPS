// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "GameplayEffectTypes.h"
#include "BrawlHealthWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UAbilitySystemComponent;

/**
 * UBrawlHealthWidget
 * 
 * 캐릭터 머리 위에 표시되는 체력바 위젯 클래스
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlHealthWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	// ASC와 연동하여 체력 변화 감지 시작
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	// 체력 변경 시 호출될 이벤트
	void OnHealthChanged(float NewValue, float MaxValue);

	// 팀 색상 등을 변경하기 위한 이벤트
	void OnTeamColorChanged(bool bIsEnemy);

private:
	// GAS 어트리뷰트 변경 콜백
	void HealthChanged(const FOnAttributeChangeData& Data);
	void MaxHealthChanged(const FOnAttributeChangeData& Data);
	
public:
	// 블루프린트에서 바인딩할 UI 요소들
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HealthText;
	
	// 체력 바 색상
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	FLinearColor HealthBarColor = FLinearColor::Green;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Brawl|UI")
	FLinearColor EnemyHealthBarColor = FLinearColor::Red;
	
private:
	// 현재 값 캐싱
	float CurrentHealth = 0.0f;
	float CurrentMaxHealth = 0.0f;
};
