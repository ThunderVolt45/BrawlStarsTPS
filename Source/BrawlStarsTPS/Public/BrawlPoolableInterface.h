// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BrawlPoolableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBrawlPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 오브젝트 풀링이 가능한 액터들이 구현해야 하는 인터페이스
 */
class BRAWLSTARSTPS_API IBrawlPoolableInterface
{
	GENERATED_BODY()

public:
	/** 풀에서 꺼내져 활성화될 때 호출 */
	virtual void OnActivate() = 0;

	/** 풀로 돌아가며 비활성화될 때 호출 (상태 리셋) */
	virtual void OnDeactivate() = 0;

	/** 현재 활성 상태인지 확인 */
	virtual bool IsActive() const = 0;

	/** 게임 시작 시 미리 생성해둘 추가 요구사항 반환 (BaseCount: 이 액터의 생성 예정 개수) */
	virtual void GetPrewarmRequirements(TMap<TSubclassOf<AActor>, int32>& OutRequirements, int32 BaseCount) const {}
};
