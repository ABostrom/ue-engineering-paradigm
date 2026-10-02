# Data assets, discovery and validation

**C++ builds the machine. Data decides what it does.** Keep pushing decisions down that
boundary until adding content needs no programmer - and stop where things merely look alike.

## The ladder

Each rung is a decision most codebases leave in C++. Walk down it and ask at each step: *does
this still need a programmer?*

| Rung | Left in C++ | Pushed into data |
|---|---|---|
| Which asset | `LoadObject(TEXT("/Game/..."))` | a `TSoftObjectPtr` / `TObjectPtr` `UPROPERTY` |
| Where assets live | a hardcoded directory or hand list | Asset Manager primary asset type + scan directories |
| Which asset *type* | `TEXT("WeaponDefinition")` literal | an `FPrimaryAssetType` `UPROPERTY` |
| Which child class | `CreateWidget<UEnemyRow>` / `AGruntEnemy::StaticClass()` | a `TSubclassOf` / `TSoftClassPtr` `UPROPERTY` |
| What the child does | a C++ `NativeConstruct` / `virtual` | `BlueprintNativeEvent` overridable in Blueprint |

The bottom two rungs are the ones usually missed, and they are the difference between
"generic" and "reusable".

**The container knows how to ask; the child knows what to draw.** A list widget knows how to
query the Asset Manager, sort, handle empty and track selection. A row knows what an asset *of
its kind* looks like. Generalise the asking; keep the showing specific.

**The stop condition is half the rule.** Do not unify things that merely look alike. A list of
weapon definitions on disk and a list of live network sessions are both "a vertical stack of
rows", but they have different lifetimes, failure models and no asset behind the second. If you
have to weaken the abstraction to fit the second case, it did not belong.

**When to make something configurable:** when you write the *second* instance. Not the third,
not "later".

## C++ names no content

Not `LoadObject`, not `ConstructorHelpers::FObjectFinder`, not an `FSoftObjectPath` built from a
string in a constructor. An asset a class needs is an `EditDefaultsOnly` property; its value is
assigned on the Blueprint child or the data asset. A missing assignment fails **loudly** (a
validation error, a log naming what will not work), never by substituting a hardcoded path,
which turns a visible content mistake into a hidden one. Do not keep a surviving violation as
precedent.

## Primary data assets and the Asset Manager

One asset per *instance of content* (one per enemy, weapon, mode), each a `UPrimaryDataAsset`
subclass. The Asset Manager finds them; nothing lists them by hand.

- `UPrimaryDataAsset` implements `GetPrimaryAssetId()`. For a native subclass the primary asset
  type is the name of the first native class up the hierarchy; for Blueprint subclasses, the
  highest Blueprint class (header comment in `Engine/Source/Runtime/Engine/Classes/Engine/DataAsset.h`).
  Override `GetPrimaryAssetId()` to pin the type explicitly.
- Register the type and its directories in **Project Settings > Asset Manager**
  (`PrimaryAssetTypesToScan` in `DefaultGame.ini`). New assets in those directories are found
  with no code or config edit.
- Query without loading: `UAssetManager::GetPrimaryAssetIdList` /
  `GetPrimaryAssetDataList`. Load asynchronously: `LoadPrimaryAssets` /
  `LoadPrimaryAssetsWithType`, optionally with bundles. All in
  `Engine/Source/Runtime/Engine/Classes/Engine/AssetManager.h`. Note the documented behaviour:
  if there is no work to do, the returned handle is null and the delegate is still called.
- Asset bundles: `meta = (AssetBundles = "Gameplay")` on soft references lets you load "the
  definition plus what play needs" in one request. The bundle scan walks into instanced structs:
  `FPropertyValueIterator` enters an `FInstancedStruct`'s payload
  (`FillStructProperties` in `Engine/Source/Runtime/CoreUObject/Private/UObject/UnrealType.cpp`),
  so do not hand-walk fragments to collect bundles.
- Identity: use the `FPrimaryAssetId` for *individuals* (this enemy). Use gameplay tags for
  *kinds* (undead, ranged).

Template: `templates/ValidatedDefinition.h/.cpp`, `templates/DefinitionDiscovery.cpp`.

**Acceptance test - run it, do not reason about it:** author one new data asset. It should
appear in play with no rebuild, no code edit and no config edit. If it does not, the reason is
always interesting: a scan directory, a cook rule, a hand list someone forgot. Find it.

## Validate at the edge, then trust

Bad or incomplete **authored** data is:

1. an **editor validation error** at authoring time, and
2. a **clear refusal with a message** at runtime, where the data enters (load, spawn, equip,
   component registration).

Never a stand-in, a "default sword", a fuzzy match or a `TryRepair()`. A fallback converts
broken content into content that *looks* fine - the bug report becomes "everyone has a sword"
three weeks later.

Past the edge, code may assume its references are valid. Use `check` / `ensure` for
invariants, not `if (!X) return;` scattered through every function. A null that reaches deep
code is a bug to fix at its source, not a case to handle.

**Where runtime checks are right:** input that is not authored data - network input from
another player, save files from older versions, user-generated content. Those cross a trust
boundary at runtime, so they are validated at runtime. A fallback is also fine when it is a
**feature** a designer chose (a starter loadout for a brand-new save), and then it is data too.

**No machinery for paths that cannot occur.** A review finding earns code only if it names a
real path in play or set-up. Otherwise it gets at most a `checkf` that states the rule.

### Two validation tools

| Tool | Use when | Where |
|---|---|---|
| `UObject::IsDataValid(FDataValidationContext&) const` | Your own class can check itself | `Engine/Source/Runtime/CoreUObject/Public/UObject/Object.h` |
| `UEditorValidatorBase` subclass | Rules across assets, or rules about classes you do not own; project-wide policies | `Engine/Plugins/Editor/DataValidation/Source/DataValidation/Public/EditorValidatorBase.h` |

`UEditorValidatorBase` header: "It should be used when checking engine-level classes, as
UObject::IsDataValid requires overriding the base class." C++ and Blueprint validators are
gathered on editor start. Override `CanValidateAsset_Implementation` and
`ValidateLoadedAsset_Implementation`; call `AssetFails` / `AssetPasses` ("Failure to call this
will report the validator as not having checked the asset"). Engine example:
`Engine/Plugins/Editor/DataValidation/Source/DataValidation/Private/PackageFileValidator.h`.

Template: `templates/EditorValidator.h/.cpp`.

### The CDO trap in `IsDataValid`

Compiling a Blueprint runs `IsDataValid` on the Blueprint **and on its class default object**
(`Engine/Source/Editor/Kismet/Private/BlueprintCompilationManager.cpp`). An actor or definition
Blueprint's CDO usually holds none of the data its *instances* hold, so a strict `IsDataValid`
refuses the CDO and the Blueprint is born in an error state - which then errors on every load
and every cook. When the content lives on **instances** (data asset instances), return
`EDataValidationResult::NotValidated` when `HasAnyFlags(RF_ClassDefaultObject)` and keep the real
rules strict for instances (`templates/ValidatedDefinition.cpp` shows this). When the content
lives on the **class default** (an ability Blueprint's tunables), validate the CDO but skip
abstract bases. See `traps.md` #4.

### Registration validators

The most valuable validator is the one that proves **content cannot silently fail to
register**: every primary asset of a type is inside a scanned directory; every ability
Blueprint is referenced by some ability set; every enemy definition's class is a Blueprint
child of the enemy base; every input action is mapped. Content that exists but is never found
is the hardest bug to see, because nothing errors - it just never shows up.

## Composable definitions with instanced structs

A definition is often best as an array of **fragments**, each one aspect of the thing:

```cpp
USTRUCT(BlueprintType) struct FEnemyFragment { GENERATED_BODY() };
USTRUCT(BlueprintType) struct FEnemyFragment_Stats : public FEnemyFragment { GENERATED_BODY() /* health, poise */ };
USTRUCT(BlueprintType) struct FEnemyFragment_Abilities : public FEnemyFragment { GENERATED_BODY() /* ability sets */ };

UPROPERTY(EditDefaultsOnly, Category = "Enemy")
TArray<TInstancedStruct<FEnemyFragment>> Fragments;
```

Systems query for the fragment types they understand and ignore the rest, so a new aspect is an
addition, not an edit to a shared struct. `TInstancedStruct` is a type-safe wrapper over
`FInstancedStruct` and "automatically defines the BaseStruct property meta-data"
(`Engine/Source/Runtime/CoreUObject/Public/StructUtils/InstancedStruct.h`). `ExcludeBaseStruct`
meta hides the empty base from the picker.

**Fragments describe what a thing is, never the order things happen in.** A fragment array
that a C++ loop *executes* step by step is a graph drawn in a dropdown - see
`abilities-as-graphs.md`, smell 8.

**Scripting boundary.** Editor scripting and tooling commonly write *fields inside* the current
struct type of an instanced struct without trouble, while *changing the struct type* through a
generic property setter can report success and change nothing. Always read the value back after
writing one. Default the slot to the right struct type in C++ when tools need to author it.

## Gameplay tag or enum: decide by the shape of the set

> When a set is **open and design-driven at both ends**, use a gameplay tag. When the system is
> **core and the set is finite**, use an enum.

Ask both halves: *could content add a member tomorrow without a programmer, and should it?*

| Set | Choice | Why |
|---|---|---|
| Enemy factions, damage types designers invent, item categories, cue tags | Tag | Open, authored, read by data |
| The ability's activation result, an action cost kind, a hit outcome the C++ resolves | Enum | Closed; C++ switches on every member |

A tag buys authorability and pays with the compiler's knowledge: if C++ has to handle every
member anyway, a tag is a stringly-typed enum with nothing checking it, and a missing case is a
silent no-op instead of a build error. **A tag family no C++ ever reads, for a closed set, is a
set that should have been an enum** - and often already is one somewhere else.

Declare native tags with `UE_DEFINE_GAMEPLAY_TAG_COMMENT` so the tag picker explains them
(`Engine/Source/Runtime/GameplayTags/Public/NativeGameplayTags.h`).

## Checklist for a data asset schema

- [ ] One asset per content instance; a primary asset type the Asset Manager scans.
- [ ] Class references are `TSoftClassPtr`/`TSubclassOf` to Blueprint children, never `StaticClass()` lists.
- [ ] No string paths, no `LoadObject` in C++.
- [ ] Aspects as instanced-struct fragments; no sequences in data.
- [ ] `IsDataValid` on the class, CDO excluded; a validator for cross-asset rules.
- [ ] Runtime refusal with a message at the entry point; no fallback.
- [ ] Tags only for open sets, enums for closed ones.
- [ ] Acceptance test run: a new asset appears with no code or config edit.
