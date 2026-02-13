#include "GameMode/BrawlSpawnPoint.h"
#include "Components/BillboardComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BrawlCharacter.h"
#include "Environment/BrawlPowerCubeBox.h"

ABrawlSpawnPoint::ABrawlSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// 항상 일관된 루트 컴포넌트 사용 (패키지 빌드 안정성)
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

#if WITH_EDITORONLY_DATA
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	if (BillboardComponent)
	{
		BillboardComponent->SetupAttachment(RootComponent);
		
		// 에디터용 아이콘 설정 (기본 Target 아이콘 사용)
		static ConstructorHelpers::FObjectFinder<UTexture2D> IconTexture(TEXT("/Engine/EditorResources/S_Target.S_Target"));
		if (IconTexture.Succeeded())
		{
			BillboardComponent->SetSprite(IconTexture.Object);
		}
		
		BillboardComponent->bIsScreenSizeScaled = true;
	}
#endif
}

bool ABrawlSpawnPoint::IsOccupied(float CheckRadius) const
{
	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	
	// Pawn(브롤러)과 WorldDynamic(상자 등) 모두 체크
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	// 자신(스폰 포인트)은 무시
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(const_cast<ABrawlSpawnPoint*>(this));

	// 브롤러 또는 상자 클래스가 있는지 체크
	return UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		GetActorLocation(),
		CheckRadius,
		ObjectTypes,
		AActor::StaticClass(), // 모든 액터 타입에 대해 쿼리 수행 (Pawn/WorldDynamic으로 필터링됨)
		IgnoreActors,
		OverlappingActors
	);
}
