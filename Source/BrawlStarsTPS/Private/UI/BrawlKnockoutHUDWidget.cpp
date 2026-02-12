#include "UI/BrawlKnockoutHUDWidget.h"
#include "Components/Image.h"
#include "BrawlCharacter.h"
#include "BrawlGameState_Knockout.h"
#include "BrawlPlayerState.h"
#include "Kismet/GameplayStatics.h"

void UBrawlKnockoutHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 배열 초기화
	Team0BrawlerIcons.Empty();
	Team0BrawlerIcons.Add(Team0_Icon0);
	Team0BrawlerIcons.Add(Team0_Icon1);
	Team0BrawlerIcons.Add(Team0_Icon2);

	Team1BrawlerIcons.Empty();
	Team1BrawlerIcons.Add(Team1_Icon0);
	Team1BrawlerIcons.Add(Team1_Icon1);
	Team1BrawlerIcons.Add(Team1_Icon2);

	RoundIcons.Empty();
	RoundIcons.Add(Round_Icon0);
	RoundIcons.Add(Round_Icon1);
	RoundIcons.Add(Round_Icon2);

	// 초기 상태 설정 (빈 라운드)
	for (UImage* RoundImg : RoundIcons)
	{
		if (RoundImg) RoundImg->SetColorAndOpacity(EmptyRoundColor);
	}

	bIconsInitialized = false;
}

void UBrawlKnockoutHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 1. 브롤러 아이콘 매핑 (최초 1회 또는 브롤러 수 변경 시)
	if (!bIconsInitialized)
	{
		InitializeIcons();
	}

	// 2. 브롤러 생존 상태 업데이트
	UpdateBrawlerStatus();

	// 3. 라운드 정보 업데이트
	UpdateRoundStatus();
}

void UBrawlKnockoutHUDWidget::InitializeIcons()
{
	TArray<AActor*> FoundBrawlers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlCharacter::StaticClass(), FoundBrawlers);

	// 브롤러가 충분히 스폰되었을 때만 초기화 진행
	if (FoundBrawlers.Num() < 2) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ABrawlCharacter* LocalChar = Cast<ABrawlCharacter>(PC->GetPawn());
	if (!LocalChar) return;

	BrawlerToIconMap.Empty();
	int32 AllyIdx = 0;
	int32 EnemyIdx = 0;

	for (AActor* Actor : FoundBrawlers)
	{
		if (ABrawlCharacter* Brawler = Cast<ABrawlCharacter>(Actor))
		{
			UImage* TargetIcon = nullptr;
			
			// 로컬 플레이어 기준으로 아군/적군 판별
			if (LocalChar->IsAlly(Brawler))
			{
				if (AllyIdx < Team0BrawlerIcons.Num())
				{
					TargetIcon = Team0BrawlerIcons[AllyIdx++];
				}
			}
			else
			{
				if (EnemyIdx < Team1BrawlerIcons.Num())
				{
					TargetIcon = Team1BrawlerIcons[EnemyIdx++];
				}
			}

			if (TargetIcon)
			{
				BrawlerToIconMap.Add(Brawler, TargetIcon);
				
				// 아이콘 이미지 설정
				if (UTexture2D* IconTexture = Brawler->GetCharacterIcon())
				{
					TargetIcon->SetBrushFromTexture(IconTexture);
				}
				TargetIcon->SetColorAndOpacity(FLinearColor::White);
			}
		}
	}

	bIconsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("KnockoutHUD: Icons Initialized. Mapped %d brawlers."), BrawlerToIconMap.Num());
}

void UBrawlKnockoutHUDWidget::UpdateBrawlerStatus()
{
	for (auto& Elem : BrawlerToIconMap)
	{
		ABrawlCharacter* Brawler = Elem.Key.Get();
		UImage* Icon = Elem.Value;

		if (Brawler && Icon)
		{
			// 사망 시 어둡게 (0.2 정도의 밝기), 생존 시 밝게
			float Brightness = Brawler->IsDead() ? 0.2f : 1.0f;
			Icon->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness, 1.0f));
		}
	}
}

void UBrawlKnockoutHUDWidget::UpdateRoundStatus()
{
	ABrawlGameState_Knockout* KGS = GetWorld()->GetGameState<ABrawlGameState_Knockout>();
	if (!KGS) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	int32 MyTeamID = -1;
	if (ABrawlPlayerState* MyPS = PC->GetPlayerState<ABrawlPlayerState>())
	{
		MyTeamID = MyPS->GetTeamID();
	}

	const TArray<int32>& Winners = KGS->GetRoundWinners();

	for (int32 i = 0; i < RoundIcons.Num(); i++)
	{
		if (!RoundIcons[i]) continue;

		if (i < Winners.Num())
		{
			int32 WinnerTeam = Winners[i];
			
			// 로컬 플레이어 팀의 승리는 무조건 블루, 적 팀의 승리는 레드
			if (WinnerTeam == MyTeamID)
			{
				RoundIcons[i]->SetColorAndOpacity(AllyWinRoundColor);
			}
			else
			{
				RoundIcons[i]->SetColorAndOpacity(EnemyWinRoundColor);
			}
		}
		else
		{
			RoundIcons[i]->SetColorAndOpacity(EmptyRoundColor);
		}
	}
}
