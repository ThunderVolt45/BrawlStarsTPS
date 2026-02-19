# 🏆 BrawlStarsTPS: 기술적 특징 및 아키텍처 요약 (Portfolio Highlights)

본 프로젝트는 모바일 탑다운 슈팅 게임 '브롤스타즈'를 3인칭 슈팅(TPS) 환경으로 재해석하며, **언리얼 엔진 5의 고급 기능과 실무 수준의 최적화 기법**을 적용하여 구현한 포트폴리오입니다.

---

## 🚀 1. 고성능 아키텍처 및 런타임 최적화

### **범용 오브젝트 풀링 서브시스템 (`UBrawlPoolSubsystem`)**
*   **시스템 개요**: `AActor` 기반의 모든 객체를 관리하는 월드 서브시스템 기반의 풀링 구조 설계.
*   **재귀적 프리워밍 (Recursive Pre-warming)**:
    *   `IBrawlPoolableInterface`를 통해 발사체뿐만 아니라 파편, 이펙트, 드롭 아이템 등 하위 요구사항을 자동으로 끝까지 추적하여 게임 시작 시 생성.
    *   런타임 중 발생하는 메모리 할당 및 가비지 컬렉션 부하를 최소화하여 60 FPS 이상의 안정적인 프레임 유지.
*   **로비 예열 시스템 (Warm-up)**: 로비 단계에서 보이지 않는 더미 캐릭터를 활용해 주요 전투 태그와 Niagara 이펙트, 오디오 리소스를 미리 캐싱하여 첫 교전 시의 히치(Hitch) 현상 제거.

### **커스텀 GameplayCue 및 리소스 관리**
*   **Hitch-Free 이펙트 로직**: `UBrawlGameplayCueManager`를 커스터마이징하여 비동기 로드 설정을 제어, 모든 전투 이펙트 리소스를 매치 시작 전에 강제 로드하도록 설계.
*   **안정적인 GC 관리**: Slate 위젯 및 전역 시스템에서 `TObjectPtr` 및 `FGCObject`를 활용한 정교한 참조 관리를 통해 메모리 누수 방지.

---

## ⚔️ 2. GAS 기반의 심화 전투 시스템

### **Gameplay Ability System (GAS) 통합**
*   **데이터 주도 설계**: `AttributeSet`과 `GameplayAbility`를 결합하여 브롤러의 체력, 탄환 수, 이동 속도 등 가변 스펙을 유연하게 관리.
*   **정밀 투사체 판정 (Anti-Tunneling)**:
    *   탄속이 빠른 투사체가 얇은 충돌체를 통과하는 문제를 해결하기 위해 `Tick` 내 **수동 Sweep 검사** 로직 구현.
    *   이전 프레임 위치 기반의 연속성 검증을 통해 타격 판정의 높은 신뢰도 확보.

### **팀 기반 전투 로직**
*   **동적 아군/적군 필터링**: `IsAlly` 체크 로직을 폭발 범위 데미지 및 소환물 AI에 적용하여 팀 기반 교전 시스템 완성.
*   **상태 가시성 최적화**: 풀링 재사용 시 팀 정보에 따라 소환물의 체력바 색상 및 위젯 상태를 즉시 갱신하는 동적 바인딩 시스템 구축.

---

## 🗺️ 3. 확장 가능한 게임 모드 및 AI 환경

### **중앙 집중식 게임 모드 관리 (`ABrawlStarsTPSGameMode`)**
*   **공통 로직 추상화**: 포이즌 존(Poison Zone), AI 스폰, 팀 기반 스포닝 등 모든 게임 모드에 공통적으로 필요한 기능을 베이스 클래스에 집약.
*   **모드 확장성**: 쇼다운(Showdown), 바운티(Bounty), 녹아웃(Knockout) 등 다양한 규칙을 최소한의 오버라이드로 추가 가능하도록 설계.

### **환경 상호작용 및 AI**
*   **전략적 은신 시스템**: 수풀(Bush) 내 캐릭터 감지 및 투명화 로직, 지형 파괴 시 발생하는 물리 기반 아이템(파워 큐브) 드롭 시스템 구현.
*   **AI 타겟팅 아키텍처**: 팀 기반 지역 점유 및 우선순위 타겟팅을 포함한 AI 로직 적용.

---

## 💎 4. 최신 기술 및 그래픽 완성도

### **차세대 업스케일링 통합 (DLSS)**
*   **NVIDIA DLSS 지원**: `UBrawlGameInstance` 레벨에서 DLSS 플러그인을 제어하여 고해상도 환경에서도 최적의 퍼포먼스 제공.
*   **UX 디테일**: Slate 기반의 심리스 로딩 화면과 `NeverStream` 텍스처 설정을 통해 레벨 전환 시 그래픽 팝인 현상 제거.

---

## 🛠 Tech Stack
*   **Engine**: Unreal Engine 5.3+
*   **Language**: C++ (Core Logic), Blueprints (UI & Data)
*   **Features**: GAS (Gameplay Ability System), World Subsystem, DLSS, Slate UI, Object Pooling, Navigation/AI.
