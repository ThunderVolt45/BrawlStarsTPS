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

## Phase 2: Combat System (GAS) (Next Focus)
- [x] **Ability System Core:**
    - [x] `UBrawlAttributeSet`: Health, Ammo, SuperCharge.
    - [x] `UBrawlGameplayAbility`: Base class with policy support.
- [x] **Combat Assets & Logic:**
    - [x] **Animation Montages:** Implement Primary Fire, Reload, and Super montages with Anim Notifies.
    - [x] **Gameplay Effects (GE):**
        - [x] `GE_Damage`: Handle Instant damage application.
        - [x] `GE_Cost_Ammo`: Deduct ammo on fire.
        - [x] `GE_Restore_Ammo`: Logic for auto/manual reload.
        - [x] `GE_Cooldown`: Manage cooldowns for Gadgets/Super.
- [x] **Ability Implementation:**
    - [x] **Primary Fire:** HitScan/Projectile logic with spread/recoil.
    - [x] **Reload Strategy:**
        - [x] Implement `GA_Reload_Auto` (Passive).
        - [ ] *Postponed:* `GA_Reload_Manual` (Active).
        - [x] **C++: Automatic activation logic for passive reload abilities.**
    - [ ] **Super:** Ultimate ability charging and execution logic. (Next Focus)
    - [ ] **Gadget:** Special ability with cooldown. (Next Focus)
- [x] **Damage & Health:**
    - [x] Implement `GameplayEffect` execution calculation (Damage vs Armor).
    - [x] **C++: DataTable based Attribute Initialization.**
    - [ ] Death and Respawn flow.

## Phase 3: Brawler Content (Colt)
- [ ] **Colt Integration:**
    - [ ] **Visuals:** Mesh, AnimBP setup.
    - [ ] **Specific Abilities:** Dual pistols fire, Bullet Storm (Super).
    - [ ] **Feedback:** Recoil, VFX, SFX via Gameplay Cues.

## Phase 4: Game Modes & UI (The "Single Player" Core)
- [ ] **UI Implementation (Next Focus):**
    - [ ] **HUD Base:** Implement `WBP_BrawlHUD` with Health, Ammo, and Super bars.
    - [ ] **C++:** Create `UBrawlUserWidget` for event-driven attribute updates.
    - [ ] **HUD Integration:** Initialize HUD via PlayerController or HUD class.
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