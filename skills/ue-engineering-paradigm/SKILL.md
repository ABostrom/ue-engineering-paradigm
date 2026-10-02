---
name: ue-engineering-paradigm
description: Use when designing or reviewing Unreal Engine gameplay code that crosses the C++/Blueprint/data boundary - adding a gameplay verb or GAS ability, designing a BlueprintCallable function or UAbilityTask, defining a data asset schema, deciding whether something is C++, a Blueprint child or a data asset, decomposing a god-class GameMode or PlayerController, or reviewing code with a type enum plus switch, a fallback for missing data, a content path in C++, or a replicated value that never arrives.
---

# The Unreal engineering paradigm

**C++ is a machine with no opinions. The Blueprint child is the grammar. Data assets are the adjectives.**

Most Unreal design mistakes are a layer violation, not a coding error. The goal of every
decision below is one falsifiable outcome: **the next piece of content (a verb, an enemy, a
mode, an item) is made in a Blueprint or a data asset, with no programmer.** When content
still needs a programmer, that is a finding.

## The three layers

| Layer | Holds | Never holds |
|---|---|---|
| **C++** | Capabilities, invariants, arithmetic, latent tasks, validation. Exposes nodes. | One verb's order. A verb's name. Content paths. |
| **Blueprint child** | The sequence: *wind up, then wait for the hit frame; if parried, stagger him, else spend stamina.* **The BP child IS the verb.** Also: meshes, montages, tunables. | Arithmetic. Anything a designer shouldn't re-derive. |
| **Data asset** | Values and choices that vary without changing the sequence: timings, costs, labels, which class, which tags. | Order. Branching. |

**The boundary test:** a number, or a choice from a set, with no order implied → **data**.
Order, branching, "then", "if he" → **graph**. Arithmetic on either → **C++**.

## Where does this go? (decision procedure)

Ask in order; stop at the first yes.

1. **Does something already hold this fact?** A list, registry or asset that already knows it,
   or a parentage the asset registry can answer? → Use or derive from it. Two things that must
   agree will drift silently. Never invent a second store. *(composition-and-god-classes.md)*
2. **Is it a computation, an invariant, or something the compiler should check?** → C++, as a
   node the graph calls. *(node-contract.md)*
3. **Is it the order of events, a wait, or a branch an author draws?** → the Blueprint graph of
   the verb's own BP child. If it waits for time to pass it is a latent `UAbilityTask` named
   `WaitFor<What>`; otherwise a plain `BlueprintCallable`. *(abilities-as-graphs.md)*
4. **Is it a value, an asset reference, or a pick from a set?** → an `EditDefaultsOnly`
   property, assigned on the Blueprint child or a data asset. Never a constructor literal,
   never a `/Game/` path in C++. *(data-assets-and-validation.md)*
5. **Is it a new kind of content (enemy, weapon, mode)?** → a primary data asset per instance,
   discovered by the Asset Manager, validated by `IsDataValid`. No hand list. *(content-velocity.md)*
6. **Is it a new job for a big class (GameMode, PlayerController, Character)?** → a component or
   subsystem the class composes, or the phase that owns it. Never the class body.
   *(composition-and-god-classes.md)*
7. **Is it an open set designers extend?** → gameplay tag. **A closed set C++ switches on?** → enum.

## One Blueprint child per verb - never a type enum plus a switch

Five attacks are **five Blueprint children with five graphs**, sharing a C++ base that
exposes nodes. Not one class with an `EAttackType` and a switch, and not a Blueprint with a
`Switch on EAttackType` node.

The enum is the config-schema trap: `{windup, damage, hitCount}` fits every attack you had
when you designed it; the first one that wants a second hit, a wait for release, or a branch
only on a perfect input grows a field and a C++ case. That is **extension, not
arrangement.** Dropdowns can only express the verbs someone already thought of.

**Falsifiable test of any node set: the next verb is an arrangement of existing nodes, not
new C++.** If authoring it needs a programmer, the node set is wrong.

### What experienced engineers still get wrong here

| Tempting design | Why it fails the test | Do instead |
|---|---|---|
| C++ `ActivateAbility` runs the tasks and callbacks; the BP child only sets the montage and data asset | An **empty wrapper**: the sequence is still in C++, so the next verb needs a programmer | The BP child's `ActivateAbility` graph draws the order; C++ supplies the nodes and does not override it |
| A data list of steps, `TArray<TInstancedStruct<FAttackStep>>`, "so designers add attacks without code" | A graph drawn in a dropdown: every new step kind and ordering rule is C++ again | Sequences go in graphs. Instanced structs compose **aspects** (what a thing *is*), never **sequences** |
| An outcome enum or struct the graph `Switch`es / `Break`s on | The author re-derives what the node knew; arithmetic follows the Branch | Exec outputs on the node: `ExpandEnumAsExecs` on a function, one delegate per outcome on a task |
| A "perfect" variant as a tier enum or `bIsPerfect` flag | Extension, not arrangement | Another BP child, or a branch drawn in the graph |
| Central hit/damage resolution in C++ | **Correct** - that is computation | Keep it; expose it as one node the graphs call |

Actors and components that content spawns or grants get a Blueprint child even when there is
nothing to tune yet - it is the authoring surface (mesh, montage, defaults). A plain
`UDataAsset` does **not** need one: the asset instance already is the tuning surface.

## The node contract (everything Blueprint can see is public API)

1. **One job, named like an API** - `<Verb><Object>[<Qualifier>]`, verb-first, no subject, no
   narration, no past tense, acronyms UPPERCASE (`HUD`, `UI`, `AI`).
2. **Inputs only what it cannot derive.** An enum the node converts into an actor is a pointer
   with hardcoded destinations - pass the actor.
3. **Outputs only what it computes.** A pin that is null half the time means the node does not
   know what it returned.
4. **Hand the result on a wire, never leave it in a field.** **The wire is the state.**
5. **Fork (exec outputs) only where the author has two different things to draw.**
6. **Nothing unsafe may be left unconnected.** Prefer a shape the compiler rejects -
   `UPARAM(Required)` or `UPARAM(ref)` - over a runtime guard.

Latent vs instantaneous, naming, signatures and the compile-error tricks: **references/node-contract.md**.

## A good C++ instinct, one layer too far out

| Good C++ instinct | At the Blueprint boundary | Do instead |
|---|---|---|
| Bundle params into a struct | Opaque pin, needs a Break node, parts invisible | Flat pins |
| Dispatch on an enum | Converted straight back into the thing it named | Pass the thing |
| Encapsulate state in members | Next node reads it behind the author's back | State on the wire |
| Return a result, caller switches | Author draws a Branch, then arithmetic | Exec outputs, only where there are two things to draw |
| `protected virtual` for extension | A Blueprint child cannot reach it | `BlueprintNativeEvent` |
| Rename freely, compiler finds callers | **Silently orphans every graph that called it** | Land C++ and graph edits together, or add a CoreRedirect |

The compiler protects a C++ refactor and abandons a Blueprint one. Never leave a window where
the C++ has moved and the graphs have not.

## Branching is the authoring

Do not hide a result to stop designers branching on it. Take branching away and the sequence
returns to C++, and every new verb needs a programmer again. Guard misuse with the node's
shape, never by narrowing what an author can express.

## Validate at the edge, then trust

Bad authored data is an **editor validation error** (`IsDataValid`, `UEditorValidatorBase`)
plus a **clear refusal** where the data enters. Never a fallback, stand-in or "repair". Past
the edge, use `check`/`ensure`, not `if (!X) return;` sprinkled everywhere. Untrusted input
(network, old saves) is different and does get runtime checks.

## Red flags - stop and re-layer

| You see | It means | Read |
|---|---|---|
| A verb's name inside a C++ symbol (`ResolveHeavyStrikeVsParry`) | A sentence living in C++ that a graph should hold | abilities-as-graphs |
| `enum E<Thing>Type` + `switch` choosing behaviour | Config-schema trap | anti-patterns-gallery |
| `ActivateAbility` with callbacks and `bWaitingFor...` flags | Flow in C++; the flags can disagree with the handlers | abilities-as-graphs |
| A struct-in, struct-out `BlueprintCallable` | Opaque pins; graph will do arithmetic | node-contract |
| `LoadObject`/`FObjectFinder`/`TEXT("/Game/...")` in C++ | Content decided by a programmer | data-assets-and-validation |
| `if (!Data) { use default; }` | A fallback hiding broken content | data-assets-and-validation |
| A hand-maintained `TArray` of types/classes | A second copy of a fact the Asset Manager holds | content-velocity |
| A data-only subclass in C++ (only a constructor) | Data pretending to be a class | abilities-as-graphs |
| A 2,000-line GameMode/Controller "because it was already there" | Jobs without a home | composition-and-god-classes |
| A client (HUD, widget) reading the GameMode, or a second cached copy of a replicated fact | GameMode is server-only; one owner per fact | composition-and-god-classes |
| A component added at runtime with no `SetIsReplicatedByDefault(true)` | Clients cannot even name it, so nothing it grants can be referenced | traps |
| Renaming/removing a `UFUNCTION`, pin or enum value graphs use | Graphs orphan silently; C++ compiles green | traps |
| "These two lists must agree" in a comment | They will drift, silently | composition-and-god-classes |
| A gameplay tag no C++ ever reads, for a closed set | An enum with no compiler | data-assets-and-validation |
| A runtime value that equals the CDO and "doesn't replicate" | Archetype baseline | traps |

## Traps that look like working code

A default standing in for a runtime value, with the wrong answer looking exactly like a right
one: container defaults frozen in saved Blueprints; `FGameplayAbilitySpec::Ability` is the
CDO; a replicated value equal to the archetype never travels; a runtime-added component that
does not replicate cannot be named on the wire; renames orphan graphs; a Blueprint CDO that
fails `IsDataValid` is born in error. **references/traps.md**.

## Reference index (load on demand)

| File | When |
|---|---|
| `references/node-contract.md` | Designing any `BlueprintCallable`, `UAbilityTask` or `BlueprintNativeEvent` |
| `references/abilities-as-graphs.md` | Building or converting a GAS ability; the smells and their conversions |
| `references/data-assets-and-validation.md` | Data asset schemas, Asset Manager, validation, instanced structs, tag vs enum |
| `references/content-velocity.md` | Auditing whether content can be added with no code |
| `references/composition-and-god-classes.md` | Thin GameMode/Controller, phases, one owner per fact |
| `references/traps.md` | CDO/default traps, replication baselines, renames, asset restore traps |
| `references/reviewing.md` | Reviewing a C++/Blueprint change, and reviewing graphs |
| `references/anti-patterns-gallery.md` | Before/after pairs for every red flag above |

## Templates (`templates/`)

`LatentAbilityTask` (WaitFor... task), `InstantNode` (plain BlueprintCallable),
`NativeEventSeam` (BlueprintNativeEvent), `RequiredPinNode` (UPARAM(Required)/ref),
`ValidatedDefinition` (UPrimaryDataAsset + IsDataValid), `EditorValidator`
(UEditorValidatorBase), `DefinitionDiscovery` (Asset Manager query and async load).
