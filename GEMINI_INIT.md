# GEMINI INITIALIZATION CONTEXT

## Project Metadata
- **Project Name:** BrawlStarsTPS
- **Engine Version:** Unreal Engine 5.7
- **Language:** C++ (Primary), Blueprint (Data/Visuals)
- **Description:** A personal portfolio project reinterpreting Brawl Stars as a High-Quality Third-Person Shooter. The project aims to implement a modular gameplay architecture inspired by the **Lyra Starter Game**, focusing on C++ proficiency.
- **Root Directory:** `C:\Users\zxc98\Documents\Unreal Projects\BrawlStarsTPS`

## Current Context
- **Status:** Phase 1 (Foundation & Architecture Setup)
- **Key Files:** 
  - `Source/BrawlStarsTPS/BrawlStarsTPSCharacter.cpp` (Base Character)
  - `BrawlStarsTPS.uproject` (Config & Plugins)
- **Plugins Detected:** StateTree, GameplayStateTree, GameplayAbilities (Required), EnhancedInput (Required)

## Documentation
- **Portfolio Milestones:** [PORTFOLIO_MILESTONES.md](./PORTFOLIO_MILESTONES.md)
- **Key File Paths:** [KEY_FILE_PATHS.md](./KEY_FILE_PATHS.md)

## Instructions for Gemini
- **Architecture:** Follow **Lyra Starter Game** patterns (Modular Game Mode, Component-Based Logic, GAS).
- **Language:** **Strict C++ First.** Implement core logic (Abilities, Movement, Rules) in C++. Expose to Blueprints only for configuration and visual binding.
- **Planning:** Always reference `PORTFOLIO_MILESTONES.md` before suggesting changes.
- **Localization:**
    - **Answer in Korean.**
    - **Code comments must be in Korean.**
- **Input:** Use `EnhancedInput` with C++ bindings (Data Assets for Input Config).

## Key Documents
- [Milestones](./PORTFOLIO_MILESTONES.md)
- [Key File Paths](./KEY_FILE_PATHS.md)
- [Brawler Template](./Docs/DESIGN_TEMPLATE_BRAWLER.md)
- [Animation Template](./Docs/DESIGN_TEMPLATE_ANIMATION.md)
