# BrawlStarsTPS — 게임 흐름도

프로젝트의 게임 진행 흐름을 **세션 라이프사이클 → 매치 상태 머신 → 인게임 전투 루프** 세 단계로 정리한 문서입니다. 모든 다이어그램은 [Mermaid](https://mermaid.js.org/) 형식이며 GitHub·VS Code에서 바로 렌더링됩니다.

> 클래스 단위 구조는 [Docs/ClassDiagrams/](ClassDiagrams/00_Overview.md)를, 핵심 파일 경로는 [KEY_FILE_PATHS.md](../KEY_FILE_PATHS.md)를 참고하세요.

---

## 1. 세션 라이프사이클 (로비 → 매치 → 결과 → 로비)

레벨 전환을 가로지르는 큰 흐름입니다. `UBrawlGameInstance`가 선택(브롤러/모드/맵)을 유지하고, 로딩 화면을 거쳐 매치 레벨로 진입한 뒤, 매치 종료 후 다시 로비(`LV_Lobby`)로 복귀합니다.

로비(`ABrawlLobbyPlayerController`)는 **허브** 역할을 합니다. 메인 로비 위젯에서 선택 창으로 진입(`ShowBrawlerSelect` / `ShowGameModeSelect`)했다가, 선택을 확정하거나 뒤로 가면 다시 메인 로비(`ShowLobby`)로 돌아옵니다. 두 선택은 순서가 정해져 있지 않으며, 시작 버튼을 누르면 `StartGame()`이 호출됩니다.

```mermaid
flowchart TD
    Start([게임 실행]) --> Lobby{{메인 로비 · 허브<br/>LobbyWidget · ShowLobby}}

    subgraph LobbyFlow[로비 흐름 · ABrawlLobbyPlayerController]
        Lobby -->|ShowBrawlerSelect| Brawler[브롤러 선택 창<br/>BrawlerSelectWidget]
        Brawler -->|선택 확정 SetSelectedBrawler / 뒤로| Lobby

        Lobby -->|ShowGameModeSelect| Mode[게임 모드·맵 선택 창<br/>GameModeSelectWidget]
        Mode -->|선택 확정 SetSelectedGameMode / 뒤로| Lobby
    end

    Lobby -->|시작 버튼 StartGame| Save[GameInstance에 선택 저장<br/>Brawler · GameMode · Map]
    Save --> Loading[로딩 화면 표시<br/>ShowLoadingScreen · Slate]
    Loading --> Open[OpenLevel SelectedMapName]

    Open --> Match[[매치 진행<br/>2. 매치 상태 머신 참조]]
    Match --> Result[결과 화면<br/>MatchResult / FinalSummary]
    Result --> ReturnLobby[OpenLevel LV_Lobby]
    ReturnLobby --> Lobby

    classDef gi fill:#1e3a5f,stroke:#4a90d9,color:#fff
    classDef act fill:#3d2f5f,stroke:#9b6fd9,color:#fff
    class Save,Loading,Open,ReturnLobby gi
    class Match,Result act
```

---

## 2. 매치 상태 머신 (`EBrawlMatchState`)

매치 레벨에 진입하면 `ABrawlStarsTPSGameMode`가 `ABrawlGameState`의 상태를 전이시키고, PlayerController에 부착된 `UBrawlMatchFlowComponent`가 그 변화에 반응해 연출·BGM을 재생합니다.

```mermaid
stateDiagram-v2
    [*] --> Waiting : 레벨 BeginPlay

    Waiting --> Intro : 팀 배정·봇 스폰·풀 프리워밍 완료
    note right of Intro
        준비 BGM 재생
        모드 안내 위젯 표시
        StartDelay 타이머 시작
    end note

    Intro --> MatchStart : StartMatch() (StartDelay 경과)
    note right of MatchStart
        "START" 연출 (1.5초)
    end note

    MatchStart --> Playing : 1.5초 후 자동 전이
    note right of Playing
        전투·포이즌 존·처치 집계
        모드별 승패 조건 평가
    end note

    Playing --> GameOver : 승패 확정<br/>(Showdown/Bounty)
    Playing --> Intermission : 라운드 종료<br/>(Knockout · 승수 미달)
    Intermission --> Playing : StartNewRound() (RoundResetDelay)
    Intermission --> GameOver : 필요 승수 달성

    GameOver --> [*] : 결과 UI → LV_Lobby 복귀
```

**모드별 종료 분기**

| 모드 | Playing 중 평가 | 종료 경로 |
|------|----------------|----------|
| **Showdown** | `CheckGameEndCondition` — 생존자 1명 | Playing → GameOver |
| **Bounty** | `CheckWinCondition` — 타임아웃/목표 점수 | Playing → GameOver |
| **Knockout** | 팀 전멸 시 라운드 종료 | Playing ⇄ Intermission → GameOver (3선 2승) |

---

## 3. 인게임 전투 루프 (Playing 상태)

`Playing` 상태에서 매 프레임/이벤트 단위로 반복되는 핵심 상호작용입니다. 입력은 `UBrawlHeroComponent`를 거쳐 GAS로 전달되고, AI는 Behavior Tree가 전략을 산출해 같은 캐릭터 베이스를 구동합니다. 발사체·이펙트는 오브젝트 풀에서 대여/반납됩니다.

```mermaid
flowchart TD
    subgraph Player[플레이어 입력]
        Input[Enhanced Input] --> Hero[HeroComponent<br/>태그→어빌리티]
        Hero --> ASC_P[ASC · GameplayAbility]
    end

    subgraph AI[AI 봇]
        BT[Behavior Tree<br/>BTS_EvaluateStrategy] --> Strat{전략 분기}
        Strat -->|순찰/이동/교전/도주| BTTask[BT Task]
        BTTask --> ASC_A[ASC · GameplayAbility]
    end

    ASC_P --> Fire[발사/슈퍼/가젯 어빌리티]
    ASC_A --> Fire

    Fire -->|GetFromPool| Pool[(PoolSubsystem)]
    Pool --> Proj[Projectile / AreaEffect]
    Proj --> Hit{명중 판정<br/>Anti-Tunneling}
    Hit -->|적중| Effect[GameplayEffect<br/>데미지 적용]
    Hit -->|소멸| Return[ReturnToPool]

    Effect --> Health[AttributeSet<br/>Health 변경]
    Health --> HUD[HUD 갱신]
    Health --> Dead{체력 0?}
    Dead -->|예| Die[Die · NotifyKill]
    Dead -->|아니오| Player

    Die --> KillFeed[KillFeed · 점수/현상금 갱신]
    KillFeed --> Respawn{ShouldRespawn?}
    Respawn -->|예| RespawnAt[RespawnAt]
    Respawn -->|아니오| EndCheck[모드 종료 조건 평가]
    RespawnAt --> Player

    classDef sys fill:#3d2f5f,stroke:#9b6fd9,color:#fff
    classDef core fill:#1e3a5f,stroke:#4a90d9,color:#fff
    class Pool,Proj,Effect,Health sys
    class Fire,Die,EndCheck core
```

---

## 관련 문서

- [01_CoreFramework.md](ClassDiagrams/01_CoreFramework.md) — GameMode / GameState / 매치 상태 enum
- [04_Components_Input.md](ClassDiagrams/04_Components_Input.md) — HeroComponent · MatchFlowComponent
- [05_UI.md](ClassDiagrams/05_UI.md) — 로비·HUD·결과 위젯 흐름
- [02_GAS_Abilities.md](ClassDiagrams/02_GAS_Abilities.md) — 어빌리티·이펙트 상세
- [03_AI.md](ClassDiagrams/03_AI.md) — Behavior Tree 전략 분기
