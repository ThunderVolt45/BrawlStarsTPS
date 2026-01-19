// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BrawlCharacterData.generated.h"

/**
 * FBrawlCharacterData
 *
 * 데이터 테이블(DataTable)에서 브롤러의 초기 능력치를 관리하기 위한 구조체입니다.
 */
USTRUCT(BlueprintType)
struct BRAWLSTARSTPS_API FBrawlCharacterData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 캐릭터 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float MaxHealth = 6000.0f;
	
	// 캐릭터 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float MoveSpeed = 600.0f;
	
	// 최대 탄환 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float MaxAmmo = 3.0f;
	
	// 재장전 대기 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float ReloadDelay = 1.0f;
	
	// 기본 공격 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float AttackDamage = 1000.0f;

	// 가젯 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float Gadget1Damage = 1500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float Gadget2Damage = 1500.0f;

	// 가젯 쿨다운 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float Gadget1Cooldown = 15.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float Gadget2Cooldown = 15.0f;

	// 궁극기 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float SuperDamage = 2000.0f;

	// 궁극기 사용 시 필요한 게이지 양
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Stats")
	float SuperCost = 100.0f;
	
	// 캐릭터 아이콘 (UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Visuals")
	TSoftObjectPtr<UTexture2D> CharacterIcon;
	
	// 가젯 아이콘 (UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Visuals")
	TSoftObjectPtr<UTexture2D> Gadget1Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Visuals")
	TSoftObjectPtr<UTexture2D> Gadget2Icon;
	
	// 스타파워 아이콘 (UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Visuals")
	TSoftObjectPtr<UTexture2D> Star1Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Visuals")
	TSoftObjectPtr<UTexture2D> Star2Icon;
	
	// 하이퍼차지 아이콘 (UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brawl|Visuals")
	TSoftObjectPtr<UTexture2D> HyperIcon;
	
};
