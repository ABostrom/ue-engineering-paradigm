---
id: 03-enemy-type-needs-code
title: Designers can't add an enemy type without code
exercises: references/content-velocity.md, references/data-assets-and-validation.md
---

## Prompt

In our UE5 action game, designers complain that a new enemy type always needs a
programmer. Today:

- `enum class EEnemyType { Grunt, Archer, Brute, Shaman }`.
- `AEnemySpawner::SpawnEnemy(EEnemyType Type)` has a switch that returns
  `AGruntEnemy::StaticClass()` etc., or for newer ones calls
  `LoadClass<AEnemyBase>(nullptr, TEXT("/Game/Enemies/BP_Shaman.BP_Shaman_C"))`.
- `UWaveDirector` has `TArray<EEnemyType> AllowedTypes = { Grunt, Archer, Brute, Shaman };`
  hardcoded in its constructor.
- Enemy stats live in one big `DT_EnemyStats` DataTable that everyone edits, and it's
  constantly in merge conflicts.
- Enemy factions ("Undead", "Bandit", "Beast") are another enum, and AI code switches on it.

Propose how to restructure this so a designer can add a fifth enemy type with no
programmer. Be concrete about classes and Unreal APIs.

## A good answer must

- Replace the enum with **one primary data asset per enemy** (a `UPrimaryDataAsset`
  definition) referencing the enemy's **Blueprint class softly** (`TSoftClassPtr`).
- **Discover** definitions through the **Asset Manager** (primary asset type + scan
  directory, `GetPrimaryAssetIdList` / `LoadPrimaryAssets`), not a hand list in C++.
- Remove content paths from C++ entirely.
- Add **validation at the edge** (`IsDataValid` on the definition, or an editor
  validator) so a half-authored enemy fails in the editor, not at runtime.
- Address the merge bottleneck: **one asset per enemy** rather than one shared table.
- Apply the **set-shape rule** to factions: an open, designer-driven set is a
  **gameplay tag**; a closed set C++ switches on stays an enum.
- State an **acceptance test**: author one new asset, and it appears in play with no
  code, rebuild or config edit.

## A good answer must not

- Suggest adding the fifth value to the enum, or a `FName -> path` map in C++.
- Keep `LoadClass` / `LoadObject` / `ConstructorHelpers` with a `/Game/` path.
- Add a "default enemy" fallback when a definition is missing.
