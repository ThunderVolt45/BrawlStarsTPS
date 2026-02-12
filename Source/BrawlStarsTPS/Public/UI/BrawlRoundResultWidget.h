#pragma once

#include "CoreMinimal.h"
#include "UI/BrawlUserWidget.h"
#include "BrawlRoundResultWidget.generated.h"

class UTextBlock;

/**
 * UBrawlRoundResultWidget
 * 
 * 라운드 종료 시 승리/패배 및 점수를 표시하는 위젯입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlRoundResultWidget : public UBrawlUserWidget
{
	GENERATED_BODY()

public:
	/** 라운드 결과 데이터 설정 */
	UFUNCTION(BlueprintCallable, Category = "Brawl|UI")
	void SetupRoundResult(bool bInIsWinner, int32 InTeam1Score, int32 InTeam2Score);

	/** 블루프린트에서 승패에 따른 연출을 처리하기 위한 이벤트 */
	UFUNCTION(BlueprintNativeEvent, Category = "Brawl|UI")
	void OnRoundResultApplied();

protected:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ResultText;

	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	bool bIsWinner;

	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	int32 Team1Score;

	UPROPERTY(BlueprintReadOnly, Category = "Brawl|UI")
	int32 Team2Score;
};
