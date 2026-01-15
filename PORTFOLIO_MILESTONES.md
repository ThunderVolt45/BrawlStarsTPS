# Portfolio Milestones: Brawl Stars TPS (Lyra-Style C++ & AI)

## Project Goal
**"Brawl Stars Reimagined (Single Player)"**: Develop a high-fidelity Third-Person Shooter where the player competes against **Advanced AI Bots** in classic Brawl Stars modes (Showdown, Gem Grab).
**Technical Focus**: 
1. **Lyra-Style Modular Architecture** (GAS, Component-Based).
2. **AI-Driven Gameplay** (Behavior Trees/State Trees mimicking player behavior).
3. **C++ Proficiency** (Strict C++ implementation for core systems).

## Phase 1: Foundation & Character Reboot (Current Focus)
- [x] **Character System Re-implementation:**
    - [x] **Clean Slate:** Create `ABrawlCharacter` from scratch (discarding old logic if necessary).
    - [x] **Base Class:** Implement `UBrawlPawnComponent` base class for modular components.
    - [ ] **Modular Components:** Implement `UBrawlHeroComponent` for input/camera handling (Lyra pattern).
    - [x] **GAS Initialization:** Implement `IAbilitySystemInterface` and bind `AbilitySystemComponent`.
- [ ] **Input Architecture:**
    - [ ] Implement `UInputConfig` DataAssets.
    - [ ] C++ Input Binding via `EnhancedInput`.
- [ ] **Core Movement:**
    - [ ] Tune `CharacterMovementComponent` for "Brawl Stars-like" snappy movement (high acceleration/friction).

## Phase 2: Combat System (GAS)
- [ ] **Ability System Core:**
    - [ ] `UBrawlAttributeSet`: Health, Ammo, SuperCharge.
    - [ ] `UBrawlGameplayAbility`: Base class with policy support.
- [ ] **Standard Abilities:**
    - [ ] **Primary Fire:** HitScan/Projectile logic with spread/recoil.
    - [ ] **Reload:** Auto-replenish mechanics.
    - [ ] **Super:** Ultimate ability charging and execution.
- [ ] **Damage & Health:**
    - [ ] Implement `GameplayEffect` for damage calculation.
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