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
- [ ] **Combat Assets & Logic:**
    - [ ] **Animation Montages:** Implement Primary Fire, Reload, and Super montages with Anim Notifies.
    - [ ] **Gameplay Effects (GE):**
        - [ ] `GE_Damage`: Handle Instant damage application.
        - [ ] `GE_Cost_Ammo`: Deduct ammo on fire.
        - [ ] `GE_Restore_Ammo`: Logic for auto/manual reload.
        - [ ] `GE_Cooldown`: Manage cooldowns for Gadgets/Super.
- [ ] **Ability Implementation:**
    - [ ] **Primary Fire:** HitScan/Projectile logic with spread/recoil.
    - [ ] **Reload Strategy:**
        - [ ] Implement `GA_Reload_Auto` (Passive) and `GA_Reload_Manual` (Active).
        - [ ] **C++: Automatic activation logic for passive reload abilities.**
    - [ ] **Super:** Ultimate ability charging and execution logic.
- [ ] **Damage & Health:**
    - [ ] Implement `GameplayEffect` execution calculation (Damage vs Armor).
    - [ ] Death and Respawn flow.

## Phase 3: Brawler Content (Colt)
- [ ] **Colt Integration:**
    - [ ] **Visuals:** Mesh, AnimBP setup.
    - [ ] **Specific Abilities:** Dual pistols fire, Bullet Storm (Super).
    - [ ] **Feedback:** Recoil, VFX, SFX via Gameplay Cues.

## Phase 4: Game Modes & AI (The "Single Player" Core)
- [ ] **AI System (Critical):**
    - [ ] **AI Controller:** `ABrawlAIController` with perception.
    - [ ] **Bot Logic:** Behavior Trees / State Trees to mimic player tactics (cover, attack, retreat, collect gems).
- [ ] **Game Modes:**
    - [ ] **Showdown:** Free-for-all logic, Power Cube spawning, Poison clouds.
    - [ ] **Gem Grab:** Gem spawning mine, countdown logic, team scoring.
- [ ] **UI:**
    - [ ] HUD (Health, Ammo, Super).
    - [ ] Game Mode specific widgets (Gem count, Players alive).

## Phase 5: Polish & Multiplayer (Bonus)
- [ ] **Polish:**
    - [ ] Hit markers, Kill feed.
    - [ ] Smooth camera transitions.
- [ ] **Multiplayer:** 
    - [ ] *Low Priority:* Refactor for replication if time permits.
    - [ ] Network Optimization.