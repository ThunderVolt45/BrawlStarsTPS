// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/GameMode/BSGameModeBase.h"
#include "UObject/ConstructorHelpers.h"

ABSGameModeBase::ABSGameModeBase()
{
	// 기본 폰 클래스를 블루프린트 에셋으로 설정
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}