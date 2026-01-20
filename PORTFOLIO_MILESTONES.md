# Portfolio Milestones: Brawl Stars TPS (Lyra-Style C++ & AI)

## Project Goal
**"Brawl Stars Reimagined (Single Player)"**: Develop a high-fidelity Third-Person Shooter where the player competes against **Advanced AI Bots** in classic Brawl Stars modes (Showdown, Gem Grab).
**Technical Focus**: 
1. **Lyra-Style Modular Architecture** (GAS, Component-Based).
2. **AI-Driven Gameplay** (Behavior Trees/State Trees mimicking player behavior).
3. **C++ Proficiency** (Strict C++ implementation for core systems).

## Phase 1: Foundation & Character Reboot (Completed)
- [x] **Character System Re-implementation:**
    - [x] **Clean Slate:** Create `ABrawlCharacter` from scratch.
    - [x] **Base Class:** Implement `UBrawlPawnComponent` base class for modular components.
    - [x] **Modular Components:** Implement `UBrawlHeroComponent` for input/camera handling (Lyra pattern).
    - [x] **GAS Initialization:** Implement `IAbilitySystemInterface` and bind `AbilitySystemComponent`.
- [x] **Input Architecture (Lyra-Style):**
    - [x] **Input Config:** Implement `UBrawlInputConfig` DataAssets for Tag-to-Action mapping.
    - [x] **Custom Input Component:** Implement `UBrawlInputComponent` for automated tag-based binding.
    - [x] **Enhanced Input:** C++ Input Binding via `EnhancedInput` and Gameplay Tags.
- [x] **Core Movement & Camera:**
    - [x] **TPS Camera:** Implement `SpringArm` & `CameraComponent` with Over-the-shoulder offset.
    - [x] **Rotation Logic:** Character Yaw synced with Controller/Camera rotation for strafing.
    - [x] **Movement Tuning:** Basic setup for snappy movement in `CharacterMovementComponent`.
    - [x] **Jump Logic:** Implement jump functionality using the new Input System.

## Phase 2: Combat System (GAS) (Completed)
- [x] **Ability System Core:**
    - [x] `UBrawlAttributeSet`: Health, Ammo, SuperCharge, HyperCharge.
    - [x] **Data-Driven Initialization:** Initialize attributes from `DT_BrawlerStats`.
- [x] **Combat Assets & Logic:**
    - [x] **Projectile System:** `ABrawlProjectile` with GAS integration and hybrid collision.
    - [x] **Damage Logic:** Negative attribute application via GE.
    - [x] **Charge Logic:** Super/Hyper accumulation on hit based on Data Table per-hit values.
- [x] **Ability Implementation (C++):**
    - [x] **Primary Fire:** `UBrawlGameplayAbility_Fire` & `Colt_Fire` (Dual wield support).
    - [x] **Reload:** `UBrawlGameplayAbility_Reload` (Auto-replenish loop).
    - [x] **Super:** `UBrawlGameplayAbility_Super` (Gauge check & consume).
    - [x] **Gadget:** `UBrawlGameplayAbility_Gadget` (Cooldown based).
    - [x] **Hypercharge:** `UBrawlGameplayAbility_Hyper` (Buff & projectile modification).

## Phase 3: Brawler Content (Colt)
- [ ] **Colt Integration:**
    - [x] **Specific Abilities:** Dual pistols fire (C++ logic done).
    - [ ] **Visuals:** Mesh, AnimBP setup (Refinement needed).
    - [ ] **Feedback:** Recoil, VFX, SFX via Gameplay Cues.

## Phase 4: Game Modes & UI (Current Focus)
- [ ] **UI Implementation:**
    - [x] **C++ Foundation:** `UBrawlUserWidget` & `UBrawlHUDWidget` with attribute delegates.
    - [ ] **HUD Design (WBP):** Create `WBP_BrawlHUD` with Health, Ammo, Super, Hyper bars. (Next Step)
    - [ ] **HUD Integration:** Initialize HUD in PlayerController.
    - [ ] **Crosshair:** Dynamic ammo display on crosshair.
- [ ] **AI System (Critical):**
    - [ ] **AI Controller:** `ABrawlAIController` with perception.
    - [ ] **Bot Logic:** Behavior Trees / State Trees to mimic player tactics (cover, attack, retreat, collect gems).
- [ ] **Game Modes:**
    - [ ] **Showdown:** Free-for-all logic, Power Cube spawning, Poison clouds.
    - [ ] **Gem Grab:** Gem spawning mine, countdown logic, team scoring.
- [ ] **Game Specific UI:**
    - [ ] Game Mode specific widgets (Gem count, Players alive).
    - [ ] Death and Victory screens.

## Phase 5: Polish & Multiplayer (Bonus)
- [ ] **Polish:**
    - [ ] Hit markers, Kill feed.
    - [ ] Smooth camera transitions.
- [ ] **Multiplayer:** 
    - [ ] *Low Priority:* Refactor for replication if time permits.
    - [ ] Network Optimization.