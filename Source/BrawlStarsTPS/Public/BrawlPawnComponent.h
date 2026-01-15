// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BrawlPawnComponent.generated.h"

/*
 * BrawlPawnComponent
 * 
 * 이 프로젝트에서 사용되는 모든 Pawn 관련 컴포넌트의 베이스 클래스
 * Lyra의 UPawnComponent 패턴을 따르며, Pawn과 컨트롤러에 쉽게 접근할 수 있는 헬퍼 함수를 제공한다
 */
UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BRAWLSTARSTPS_API UBrawlPawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBrawlPawnComponent(const FObjectInitializer& ObjectInitializer);

	// 소유한 Pawn을 특정 타입으로 Casting하여 반환
	template <class T>
	T* GetPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, 
			"'T' must be a derivation of APawn");
		
		return Cast<T>(GetOwner());
	}
	
	// 소유한 Pawn을 반환
	APawn* GetPawn() const;
	
	// 소유한 Pawn의 컨트롤러를 특정 타입으로 Casting하여 반환
	template <class T>
	T* GetController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, 
			"'T' must be a derivation of AController");
		
		if (APawn* Pawn = GetPawn())
		{
			return Cast<T>(Pawn->GetController());
		}
		
		return nullptr;
	}
	
	// 소유한 컨트롤러를 반환
	AController* GetController() const;
};
