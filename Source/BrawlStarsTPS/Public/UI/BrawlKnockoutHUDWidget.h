#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlKnockoutHUDWidget.generated.h"

class UImage;
class ABrawlCharacter;

/**
 * UBrawlKnockoutHUDWidget
 * 
 * 녹아웃 모드 전용 상황판 UI 위젯
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlKnockoutHUDWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	/** 팀 브롤러 아이콘 및 매핑 초기화 */
	void InitializeIcons();

	/** 매 틱마다 브롤러 생존 상태 확인 및 아이콘 업데이트 */
	void UpdateBrawlerStatus();

	/** 라운드 승리 현황 아이콘 업데이트 */
	void UpdateRoundStatus();

protected:
	// 팀 0 (레드) 브롤러 아이콘
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Team0_Icon0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Team0_Icon1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Team0_Icon2;

	// 팀 1 (블루) 브롤러 아이콘
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Team1_Icon0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Team1_Icon1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Team1_Icon2;

	// 라운드 결과 아이콘
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Round_Icon0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Round_Icon1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Round_Icon2;

	// 라운드 아이콘 색상 설정
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Knockout")
	FLinearColor EmptyRoundColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.5f);
	
	// 플레이어 팀 아이콘 색상 설정
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Knockout")
	FLinearColor AllyWinRoundColor = FLinearColor::Blue;
	
	// 적 팀 아이콘 색상 설정
	UPROPERTY(EditDefaultsOnly, Category = "Brawl|Knockout")
	FLinearColor EnemyWinRoundColor = FLinearColor::Red;

private:
	/** 내부 관리를 위한 배열 */
	UPROPERTY()
	TArray<TObjectPtr<UImage>> Team0BrawlerIcons;
	UPROPERTY()
	TArray<TObjectPtr<UImage>> Team1BrawlerIcons;
	UPROPERTY()
	TArray<TObjectPtr<UImage>> RoundIcons;

	/** 브롤러와 아이콘 매핑 (매 라운드 초기화) */
	TMap<TWeakObjectPtr<ABrawlCharacter>, TObjectPtr<UImage>> BrawlerToIconMap;

	/** 아이콘 초기화 완료 여부 */
	bool bIconsInitialized = false;
};
