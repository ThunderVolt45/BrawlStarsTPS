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

## Phase 2: Combat System (GAS) (Completed & Refined)
- [x] **Ability System Core:**
    - [x] `UBrawlAttributeSet`: Health, Ammo, SuperCharge, HyperCharge, **MovementSpeed**, **DamageReduction**.
    - [x] **Data-Driven Initialization:** Initialize attributes from `DT_BrawlerData` (including ReloadSpeed, MoveSpeed).
- [x] **Combat Assets & Logic:**
    - [x] **Projectile System:** `ABrawlProjectile` with GAS integration and hybrid collision.
    - [x] **Damage Logic:** `IncomingDamage` meta-attribute with Defense (`DamageReduction`) calculation.
    - [x] **Charge Logic:** Super/Hyper accumulation on hit based on Data Table per-hit values.
- [x] **Ability Implementation (C++):**
    - [x] **Primary Fire:** `UBrawlGameplayAbility_Fire` (Tag-based interaction with Reload).
    - [x] **Reload:** `UBrawlGameplayAbility_Reload` (Auto-replenish loop with **Pause/Resume** logic during fire).
    - [x] **Super:** `UBrawlGameplayAbility_Super` (Gauge check & consume).
    - [x] **Gadget:** `UBrawlGameplayAbility_Gadget` (Cooldown based).
    - [x] **Hypercharge:** `UBrawlGameplayAbility_Hyper` (Buff duration & gauge reset logic).

## Phase 3: Brawler Content (Colt, Shelly, Spike)
- [ ] **Colt Integration:**
    - [x] **Specific Abilities:** Dual pistols fire (C++ logic done).
    - [ ] **Visuals:** Mesh, AnimBP setup (Refinement needed).
    - [ ] **Feedback:** Recoil, VFX, SFX via Gameplay Cues.
- [ ] **Shelly Integration:**
    - [ ] **Specific Abilities:** Shotgun Spread Fire (Multiple projectiles).
    - [ ] **Super:** Super Shell (Terrain destruction, Knockback).
    - [ ] **Visuals:** Mesh, AnimBP setup.
- [ ] **Spike Integration:**
    - [ ] **Specific Abilities:** Needle Grenade (Split on impact/max range).
    - [ ] **Super:** Stick Around (Slow field + Damage).
    - [ ] **Visuals:** Mesh, AnimBP setup.

## Phase 4: Game Modes & UI (Significantly Progressed)
- [x] **UI Implementation:**
    - [x] **C++ Foundation:** `UBrawlUserWidget` & `UBrawlHUDWidget` with attribute delegates.
    - [x] **Skill Widgets:** Specialized widgets for Gadget (Cooldown), Super (Flash/Ready), Hyper (Duration/Active).
    - [x] **HUD Integration:** `WBP_BrawlHUD` with Health, Ammo, Skill Widgets, and **Match Timer**.
    - [x] **Controller Setup:** Automated HUD creation & GAS binding in `PlayerController`.
    - [ ] **Crosshair:** Dynamic ammo display on crosshair.
- [ ] **Map Elements (Obstacles):**
    - [ ] **Destructible Walls:**
        - [ ] Block Movement & Sight (AI Visibility).
        - [ ] Destruction logic via specific Projectiles (Super/Gadget).
    - [ ] **Bushes (Grass):**
        - [ ] Allow Movement but Block Sight (Stealth mechanic).
        - [ ] AI Perception handling for Hiding.
- [ ] **AI System (Critical - Next Step):**
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
