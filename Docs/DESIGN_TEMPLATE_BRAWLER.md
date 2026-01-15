# Character Implementation Template: Brawler (TPS Ver.)

This document outlines the standardized architecture for implementing any Brawler character (Colt, Shelly, Spike, etc.) within the **Lyra-Lite** framework.

## 1. Architecture Overview (The "Brawler" Pattern)
Every Brawler follows a strict data-driven pattern rooted in `BrawlPawnData`.

```mermaid
graph TD
    A[DA_Hero_[BrawlerName] <br/> (BrawlPawnData)] --> B[B_Hero_[BrawlerName] <br/> (Pawn Class)]
    A --> C[IMC_[BrawlerName] <br/> (Input Config)]
    A --> D[AbilitySet_[BrawlerName] <br/> (GAS)]
    A --> E[CameraMode_TPS_Default <br/> (Shared Camera)]
    
    B --> F[Skeletal Mesh <br/> (Unique Model)]
    B --> G[BrawlHeroComponent <br/> (Shared Logic)]
    B --> H[CMC <br/> (Archetype Specific Tuning)]
```

## 2. Component Template

### A. Pawn Data (`DA_Hero_[BrawlerName]`)
*   **Naming Convention:** `DA_Hero_[BrawlerName]` (e.g., `DA_Hero_Colt`)
*   **Location:** `Content/Characters/[BrawlerName]/Data`
*   **Base Class:** `BrawlPawnData` (Inherits from `UPrimaryDataAsset`)
*   **Configuration:**
    *   `PawnClass`: Link to `B_Hero_[BrawlerName]`
    *   `AbilitySets`: 
        *   `AbilitySet_Brawler_Common` (Movement, Jump, Interaction)
        *   `AbilitySet_[BrawlerName]` (Primary Fire, Super, Gadget)
    *   `InputConfig`: `InputData_Hero` (Standard) or `InputData_[BrawlerName]` (Custom)
    *   `DefaultCameraMode`: `CM_TPS_Brawl` (Shared standard view)

### B. Character Blueprint (`B_Hero_[BrawlerName]`)
*   **Naming Convention:** `B_Hero_[BrawlerName]`
*   **Location:** `Content/Characters/[BrawlerName]`
*   **Parent Class:** `BP_Hero_Base` (Inherits from `ABrawlCharacter`)
*   **Key Adjustments:**
    1.  **Mesh:** Assign unique Skeletal Mesh (`SK_[BrawlerName]`).
    2.  **Animation:** Assign `ABP_[BrawlerName]` (Retargeted from Manny).
    3.  **Movement Component (CMC):**
        *   **Tank Archetype (e.g., Shelly):** Lower Speed, High Mass.
        *   **Speedster Archetype (e.g., Colt):** High Speed, High Acceleration.
        *   **Thrower Archetype:** Moderate Speed.

### C. Abilities & Weapons (`AbilitySet_[BrawlerName]`)
Instead of hardcoding logic in the Pawn, use Gameplay Ability System (GAS).
*   **Primary Weapon:** Implemented as a `BrawlWeaponInstance` + `GameplayAbility`.
    *   *Colt:* Dual Pistols (Rapid fire, straight line).
    *   *Shelly:* Shotgun (Spread trace).
    *   *Spike:* Projectile (Explodes on impact).
*   **Super Ability:** A separate `GameplayAbility` triggered by the 'Ult' key (e.g., Q or Button).

## 3. Implementation Checklist (For New Brawlers)

1.  [ ] **Asset Import:** Import Mesh (`SK_[Name]`) and Animations.
2.  [ ] **Retargeting:** Create `RTG_Manny_to_[Name]` and generate Animation Blueprint (`ABP_[Name]`).
3.  **Pawn Creation:**
    *   [ ] Duplicate `BP_Hero_Base` -> Rename to `B_Hero_[Name]`.
    *   [ ] Set Mesh and AnimBP.
    *   [ ] Tune CharacterMovementComponent values (Speed, Friction).
4.  **Data Asset:**
    *   [ ] Create `DA_Hero_[Name]`.
    *   [ ] Assign the new Pawn Class.
    *   [ ] Assign shared and specific Ability Sets.
5.  **Validation:**
    *   [ ] Test spawn in `L_ShooterGym` (or custom map) by overriding Experience settings.

## 4. Shared Resources (Do not duplicate)
*   **Camera Mode:** `CM_TPS_Brawl` (Unless the brawler requires a unique view).
*   **Common Abilities:** `AbilitySet_Brawler_Common` (Jump, Crouch, Interact).
*   **Input Config:** `InputData_Hero` (Unless unique inputs are needed).
