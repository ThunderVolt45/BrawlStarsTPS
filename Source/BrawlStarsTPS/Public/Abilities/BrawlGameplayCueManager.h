// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "BrawlGameplayCueManager.generated.h"

/**
 * UBrawlGameplayCueManager
 * 
 * GAS의 GameplayCue 로딩 및 실행을 최적화하기 위한 커스텀 매니저입니다.
 */
UCLASS()
class BRAWLSTARSTPS_API UBrawlGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	/** 
	 * GameplayCue 라이브러리를 비동기로 로드할지 여부를 반환합니다. 
	 * false를 반환하면 시작 시 모든 큐 에셋을 동기적으로 로드하여 게임 중 Hitch를 방지합니다.
	 */
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override { return false; }
};
