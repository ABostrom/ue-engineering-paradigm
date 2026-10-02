# Content velocity: can this be added with no code?

Judge every refactor by one question: **does it make the next piece of content cheaper to make
in a Blueprint or a data asset, with no C++?** When content still needs a programmer, that is a
finding. Prefer extension by addition (a new asset, a new Blueprint child) over editing a switch
or a shared file.

## Running the audit

List every *kind* of content your game will want more of. For each, try to add one (or walk
through adding one honestly) and record what it took.

| Content kind | Steps to add one | Needs code? | Needs config? | Shared file edited? | Blocker |
|---|---|---|---|---|---|
| Enemy | new `DA_Enemy_*` + `BP_Enemy_*` | no | no | no | - |
| Melee verb (Parry, Dodge, Heavy Strike...) | new `BP_GA_*` child, add to an ability set | ? | ? | ? | |
| Weapon | | | | | |
| Game mode / ruleset | | | | | |
| Match phase | | | | | |
| HUD element | | | | | |
| Level / arena preset | | | | | |
| Item / loot table | | | | | |

The answer for each row should be "no / no / no". Every "yes" names a blocker from the list below.

## The usual blockers, and their fixes

### 1. A C++ `virtual` where a data field belongs

```cpp
virtual bool IsRanged() const { return false; }   // overridden in AArcherEnemy
```

Behaviour that varies per content item as a *value* is a property, not an override.
**Fix:** `UPROPERTY(EditDefaultsOnly) bool bIsRanged;` or, if it classifies, a gameplay tag. Keep
`virtual` (or `BlueprintNativeEvent`) for genuinely different *computation*.

### 2. A hard-coded list

```cpp
AllowedEnemies = { EEnemyType::Grunt, EEnemyType::Archer, EEnemyType::Brute };
```

A second copy of a fact the content already holds. **Fix:** discover by asset
(`UAssetManager::GetPrimaryAssetIdList`), filter by tags read from the asset registry, or let
each asset opt in with a field. If a list is truly a designer's choice (this wave's roster), it
lives in a data asset, not a constructor.

### 3. `StaticClass()` / class lookups

```cpp
case EEnemyType::Grunt: return AGruntEnemy::StaticClass();
```

A programmer chooses which class content uses. **Fix:** a `TSoftClassPtr<AEnemyBase>` on the
definition pointing at a Blueprint child. C++ never names a concrete content class.

### 4. Classification by type check

```cpp
if (Ability->IsA<UGA_HeavyStrike>() || Ability->IsA<UGA_ChargedStrike>()) { ShowInStrikePanel(); }
```

**Fix:** classification as data - a tag on the ability's Blueprint child
(`Ability.Kind.Strike`) - and the panel asks for the tag. The panel then grows itself.

### 5. One shared asset everyone edits

One `DT_EnemyStats` DataTable, one `DA_AllAbilities`, one giant level Blueprint. Every change
is a merge conflict on a binary file, so content work serialises behind one lock.
**Fix:** one asset per content instance, discovered by the Asset Manager. When a list really is
needed (an ability set granted to a character), **layer** it: a base set plus addition sets
(`DefaultAbilitySets` + per-class + per-mode), each a separate asset, granted in order. Two
people adding two verbs then touch two files.

### 6. An ability or rule that is not a node

The new verb needs one wait or one action that no node provides. **Fix:** add the node (a
whole verb phrase, `node-contract.md`), not a bespoke ability class. Next time the node exists.

### 7. A listener that only C++ can write

The game announces events (round started, enemy died, parry landed) only through C++ delegates
or virtuals. **Fix:** a latent "wait for game event" node or a `BlueprintAssignable` delegate on
the owning component, so a Blueprint (a tutorial, an achievement, a HUD widget) can react without
a programmer. GAS's `WaitGameplayEvent` task is the model.

### 8. Rules as a switch on a mode enum

```cpp
if (Mode == EMode::SuddenDeath) { RespawnDelay = 0; } else if (Mode == EMode::Pit) { ... }
```

**Fix:** the mode is a primary data asset whose rules are a
`TArray<TInstancedStruct<FMatchRule>>` (or a list of components / phase classes to compose).
A new mode is a new asset.

### 9. Phases or states hardcoded as classes in code

`if (Phase == Warmup) ... else if (Phase == Round) ...` in a 2,000-line GameMode. **Fix:** each
phase is a class (or ability) named by data; the mode asset lists which phases run. See
`composition-and-god-classes.md`.

### 10. Content that can fail to register silently

The asset exists, but it is outside a scan directory, missing from a set, or of the wrong
parent class. **Fix:** a registration validator (`data-assets-and-validation.md`) that turns
"never shows up" into an editor error.

### 11. Test or debug harness inside game code

A scenario driver, a cheat-granted test ability, a "drill mode" inside gameplay classes. It
makes those classes harder to change and ships code nobody plays. **Fix:** move it to a test
module or a `UCheatManager` extension; content authors should never see it.

## How to tell the audit is done

Pick the two most common content kinds and have someone who is not a programmer add one of
each, start to finish, in play. Watch where they stop. That stop is the next fix.
