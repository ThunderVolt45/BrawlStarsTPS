# 🎯 BrawlStarsTPS: Project Mandates & Portfolio Strategy

이 문서는 Gemini CLI가 이 프로젝트에서 작업을 수행할 때 준수해야 할 **최상위 지침서**입니다. 본 프로젝트는 고품질 TPS로 재해석된 브롤스타즈 모작이며, 기술적 역량을 증명하기 위한 **개인 포트폴리오**입니다.

---

## 🏗️ 1. 핵심 아키텍처 및 설계 철학

### **데이터 주도 설계 (Data-Driven Design)**
- **입력 (Input)**: `EnhancedInput`을 사용하며, 입력의 정의와 바인딩은 데이터 에셋(`IMC`, `IA`)으로 관리합니다.
- **로직 (Logic)**: **GAS(Gameplay Ability System)**를 핵심으로 하며, `GameplayTag`를 통해 모든 상태와 이벤트를 브로드캐스팅합니다.
- **표현 (Visual)**: `GameplayCue`와 `Niagara`를 활용하여 전투 로직과 시각적 피드백을 완전 격리합니다.
- **Lyra Pattern**: Lyra Starter Game의 모듈형 게임 모드 및 컴포넌트 기반 설계를 지향합니다.

### **고성능 최적화 전략**
- **오브젝트 풀링 (`UBrawlPoolSubsystem`)**: 모든 액터 스폰은 풀링 시스템을 거치며, **재귀적 프리워밍(Recursive Pre-warming)**을 통해 런타임 히치를 방지합니다.
- **리소스 캐싱**: `UBrawlGameplayCueManager`를 통해 매치 시작 시 전투 리소스를 동기 로딩하며, 로비 단계의 **Warm-up 시스템**을 유지합니다.

---

## 💻 2. 개발 및 코딩 가이드라인

### **언어 및 스타일**
- **Strict C++ First**: 핵심 로직(어빌리티, 이동, 게임 규칙)은 반드시 C++로 구현합니다. 블루프린트는 데이터 에셋 및 UI 연결 용도로만 사용합니다.
- **언어 설정**: **모든 답변은 한국어로**, **코드 주석 역시 한국어로** 작성합니다.
- **오류 처리**: 필수 객체의 Null 체크 실패 시 에러를 "삼키지(swallow)" 않습니다. `check()` 및 `CastChecked()`를 적극적으로 사용하여 문제를 조기에 발견합니다.

### **전투 시스템 상세**
- **정밀 판정**: 빠른 탄속의 투사체는 `Tick` 내 **수동 Sweep 검사**를 통해 터널링 현상을 방지합니다.
- **팀 로직**: `IsAlly` 체크를 통해 아군/적군 필터링 및 소환물 AI 로직을 엄격히 관리합니다.

---

## 🎨 3. 비주얼 및 UX 가이드라인
- **토이 스타일 (Stylized)**: 원작의 미학을 재현하기 위해 **Roughness = 1, Metallic = 0** 설정을 기본 전략으로 취하며, 이는 시각적 시인성(Readability) 확보를 위함입니다.
- **차세대 기술**: `UBrawlGameInstance`를 통한 **NVIDIA DLSS** 통합 상태를 유지합니다.

---

## 📂 4. 문서화 및 관리
- **작업 기록**: 새로운 기능 구현 시 `Docs/` 폴더에 작업 로그를 기록하고, `PORTFOLIO_HIGHLIGHTS.md`를 최신화합니다.
- **참조 문서**:
    - [Milestones](./PORTFOLIO_MILESTONES.md)
    - [Key File Paths](./KEY_FILE_PATHS.md)
    - [Brawler Template](./Docs/DESIGN_TEMPLATE_BRAWLER.md)

---

## 🤖 Gemini를 위한 특이사항
- 작업을 제안하기 전에 항상 `PORTFOLIO_HIGHLIGHTS.md`와 `GEMINI.md`를 참조하여 프로젝트의 기술적 방향성과 일치하는지 확인하십시오.
- 모든 제안은 "왜 이 방식이 포트폴리오로서 가치가 있는지"에 대한 기술적 근거를 포함해야 합니다.
