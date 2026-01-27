# Key File Paths

## 1. Core Framework (Brawl Stars TPS)
*   **Game Mode:** `Source/BrawlStarsTPS/Public/GameMode/BSGameModeBase.h` (.cpp)
*   **Base Character:** `Source/BrawlStarsTPS/Public/BrawlCharacter.h` (.cpp)
*   **Player Controller:** `Source/BrawlStarsTPS/BrawlStarsTPSPlayerController.h` (.cpp)
*   **Project Module:** `Source/BrawlStarsTPS/BrawlStarsTPS.h` (.cpp)

## 2. Gameplay Ability System (GAS)
*   **Ability System Component:** `Source/BrawlStarsTPS/Public/BrawlAbilitySystemComponent.h` (.cpp)
*   **Attribute Set:** `Source/BrawlStarsTPS/Public/BrawlAttributeSet.h` (.cpp)
*   **Base Ability:** `Source/BrawlStarsTPS/Public/Abilities/BrawlGameplayAbility.h` (.cpp)
*   **Specific Abilities:**
    *   **Fire:** `Source/BrawlStarsTPS/Public/Abilities/BrawlGameplayAbility_Fire.h` (.cpp)
    *   **Reload:** `Source/BrawlStarsTPS/Public/Abilities/BrawlGameplayAbility_Reload.h` (.cpp)
    *   **Super:** `Source/BrawlStarsTPS/Public/Abilities/BrawlGameplayAbility_Super.h` (.cpp)
    *   **Hypercharge:** `Source/BrawlStarsTPS/Public/Abilities/BrawlGameplayAbility_Hyper.h` (.cpp)
    *   **Gadget:** `Source/BrawlStarsTPS/Public/Abilities/BrawlGameplayAbility_Gadget.h` (.cpp)
    *   **Colt Specific:** `Source/BrawlStarsTPS/Public/Abilities/Colt/BrawlGameplayAbility_Colt_Fire.h` (.cpp)
    *   **Fire & Spread Fire (Base):** `Source/BrawlStarsTPS/Public/Abilities/BrawlGameplayAbility_Fire.h` (.cpp) - Handles both single and multi-projectile (shotgun) attacks.
    *   **Auto Heal (Passive):** `Source/BrawlStarsTPS/Public/Abilities/BrawlGameplayAbility_AutoHeal.h` (.cpp)
*   **Projectile:** 
    *   `Source/BrawlStarsTPS/Public/BrawlProjectile.h` (.cpp)
    *   `Source/BrawlStarsTPS/Public/Projectiles/BrawlProjectile_Explosive.h` (.cpp)

## 3. Components & Input
*   **Hero Component (Input/Camera):** `Source/BrawlStarsTPS/Public/Components/BrawlHeroComponent.h` (.cpp)
*   **Pawn Component (Base):** `Source/BrawlStarsTPS/Public/BrawlPawnComponent.h` (.cpp)
*   **Input Component:** `Source/BrawlStarsTPS/Public/Input/BrawlInputComponent.h` (.cpp)
*   **Input Config Data:** `Source/BrawlStarsTPS/Public/Input/BrawlInputConfig.h` (.cpp)

## 4. User Interface (UI)
*   **HUD Widget:** `Source/BrawlStarsTPS/Public/UI/BrawlHUDWidget.h` (.cpp)
*   **Health Bar (Overhead):** `Source/BrawlStarsTPS/Public/UI/BrawlHealthWidget.h` (.cpp)
*   **User Widget Base:** `Source/BrawlStarsTPS/Public/UI/BrawlUserWidget.h` (.cpp)
*   **Skill Widgets:**
    *   `Source/BrawlStarsTPS/Public/UI/BrawlSkillWidget.h` (.cpp)
    *   `Source/BrawlStarsTPS/Public/UI/BrawlSuperWidget.h` (.cpp)
    *   `Source/BrawlStarsTPS/Public/UI/BrawlHyperWidget.h` (.cpp)
    *   `Source/BrawlStarsTPS/Public/UI/BrawlGadgetWidget.h` (.cpp)

## 5. Data & Configuration
*   **Character Data:** `Source/BrawlStarsTPS/Public/Data/BrawlCharacterData.h` (.cpp)
*   **Project Config:** `BrawlStarsTPS.uproject`
*   **Default Engine:** `Config/DefaultEngine.ini`
*   **Default Input:** `Config/DefaultInput.ini`
*   **Default Game:** `Config/DefaultGame.ini`
