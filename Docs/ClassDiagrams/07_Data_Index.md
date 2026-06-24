# 클래스 다이어그램 — 7. Data & Types

데이터 주도 설계의 기반인 DataTable 행 구조체(`FTableRowBase` 상속), 공용 Enum, 그리고 데이터 테이블을 소비하는 브롤러 미리보기 액터입니다.

> 위치: `Source/BrawlStarsTPS/Public/Data/`, `Public/BrawlBrawlerPreview.h`

---

## 7.1 DataTable 행 구조체 & Enum

```mermaid
classDiagram
    class FTableRowBase
    class FBrawlCharacterData {
        <<DataTable Row>>
        +MaxHealth : float
        +MoveSpeed : float
        +MaxAmmo : float
        +ReloadDelay : float
        +AttackDamage : float
        +Gadget1Damage / Gadget2Damage : float
        +Gadget1Cooldown / Gadget2Cooldown : float
        +SuperDamage : float
    }
    class FBrawlerClassData {
        <<DataTable Row>>
        +BrawlerClass : TSubclassOf~ABrawlCharacter~
        +PreviewActorClass : TSubclassOf~AActor~
    }
    class FBrawlGameModeData {
        <<DataTable Row>>
        +ModeIcon : TSoftObjectPtr~UTexture2D~
        +ModeName : FText
        +MapName : FName
        +MapDisplayName : FText
    }
    class FBrawlAIData {
        <<DataTable Row>>
        +CombatBehaviorTree : UBehaviorTree
        +MaxCombatRange : float
        +PreferredCombatRange : float
        +MinCombatRange : float
        +FleeHealthRatio : float
    }
    class EBrawlGameModeType {
        <<enumeration>>
        None / Showdown / GemGrab
        BrawlBall / Heist / Bounty / KnockOut
    }
    class EBrawlCharacterType {
        <<enumeration>>
        Hero
        Summon
        Etc
    }
    class EBrawlSpawnPointType {
        <<enumeration>>
        Brawler
        PowerCubeBox
    }

    FTableRowBase <|-- FBrawlCharacterData
    FTableRowBase <|-- FBrawlerClassData
    FTableRowBase <|-- FBrawlGameModeData
    FTableRowBase <|-- FBrawlAIData
```

---

## 7.2 데이터 소비 관계

각 데이터 테이블이 어느 클래스에서 사용되는지 보여줍니다.

```mermaid
classDiagram
    class ABrawlBrawlerPreview {
        +UpdatePreview(RowName) void
        +OnBrawlerChanged(RowName) void
        -SpawnedPreviewActor : AActor
        -CaptureActor : ASceneCapture2D
        -BrawlerClassDataTable : UDataTable
    }

    ABrawlCharacter ..> FBrawlCharacterData : 스탯 초기화
    ABrawlCharacter ..> FBrawlAIData : AI 전투 설정
    ABrawlStarsTPSGameMode ..> FBrawlerClassData : ID→클래스 매핑
    UBrawlGameInstance ..> FBrawlGameModeData : 모드/맵 정보
    ABrawlBrawlerPreview ..> FBrawlerClassData : 프리뷰 액터 스폰
    ABrawlSpawnPoint ..> EBrawlSpawnPointType : 스폰 종류 구분
```

---

### 관련 모듈
- 데이터로 초기화되는 캐릭터·게임모드·게임인스턴스 → [01_CoreFramework.md](01_CoreFramework.md)
- `FBrawlAIData`를 사용하는 AI → [03_AI.md](03_AI.md)
