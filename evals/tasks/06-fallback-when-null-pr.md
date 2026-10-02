---
id: 06-fallback-when-null-pr
title: A "fallback when null" PR
exercises: references/data-assets-and-validation.md, references/reviewing.md
---

## Prompt

A teammate's PR to our UE5 action game, titled "Make weapons robust":

1. In `UWeaponComponent::Equip`, if the character's `WeaponDefinition` (a
   `UPrimaryDataAsset`) is null, it now does
   `WeaponDefinition = LoadObject<UWeaponDefinition>(nullptr, TEXT("/Game/Weapons/DA_DefaultSword.DA_DefaultSword"));`
   and logs a warning.
2. If the definition's `HitMontage` is null, it skips the animation and applies damage
   immediately.
3. It adds `if (!WeaponDefinition) return;` to twelve other functions in the component.
4. It adds a `TryRepairDefinition()` helper that fills missing fields with defaults.

Review it. What do you approve, what do you push back on, and what should they do instead?

## A good answer must

- Push back on the fallback, the content path in C++, the scattered null checks and the
  repair helper.
- Replace them with **validation at the edge**: `IsDataValid` on `UWeaponDefinition`
  (and/or an editor validator), plus a **clear runtime refusal** where the data enters
  (equip/load), naming what is missing.
- Past the edge, **trust**: use `check`/`ensure` for invariants instead of silent skips.
- Note that a fallback **hides broken content**: the bug becomes "everyone has a sword".
- Allow runtime checks where input is genuinely untrusted (network input, save files from
  older versions), and say why that's different from authored data.

## A good answer must not

- Approve the `LoadObject` fallback, even "temporarily".
- Recommend more defensive null checks as the main fix.
- Suggest a different hardcoded default.
