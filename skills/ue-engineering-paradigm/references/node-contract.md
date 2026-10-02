# The node contract

Everything Blueprint can see is public API: node names, pin names, `Category` strings, enum
values, exposed struct fields. A Blueprint graph is a program written against that API by
someone who cannot open your `.cpp`. Design it the way you would design a library.

Running example: a melee action game with three verbs - **Parry**, **Dodge** and **Heavy
Strike** - each a GAS ability whose Blueprint child draws the sequence from C++ nodes.

## Two kinds of node

A node is either **latent** (time passes inside it) or **instantaneous** (it returns before the
next node runs). Pick by asking *does the verb wait here?* Choosing wrong in either direction is
how a node library goes bad.

| | Latent | Instantaneous |
|---|---|---|
| Built as | `UAbilityTask` subclass with a static factory | Plain `UFUNCTION(BlueprintCallable)` on the ability base (or a function library) |
| Outputs | `BlueprintAssignable` dynamic multicast delegates, one per outcome (each becomes an exec pin) | Return value / out params; `ExpandEnumAsExecs` for exec forks |
| Lifetime | Lives until it ends or the ability ends; cleanup in `OnDestroy` | None |
| Named | `WaitFor<What>` - for what the verb is waiting on | `<Verb><Object>[<Qualifier>]` |
| Examples | `WaitForIncomingHit`, `WaitForParryWindow`, `WaitForHitFrame`, `WaitForInputRelease` | `ApplyStagger`, `SpendStamina`, `OpenRiposte`, `ResolveHit` |

A verb waits on very few kinds of thing. In a melee game: an input, an animation frame, an
incoming hit, a timer. That is roughly how many tasks you should have. If you are writing the
eleventh task, check whether ten of them are the same wait with a different subject; give it a
parameter instead (`WaitForHit(Subject: Self | Target)`).

How the engine wires a latent task: the task header documents the contract - BlueprintAssignable
delegates are the outputs, the static factory's parameters are the inputs, and `Activate()` starts
the work (`Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/Tasks/AbilityTask.h`).
The Blueprint node for a gameplay task calls the factory and then `ReadyForActivation()` for you
(`ProxyActivateFunctionName` in `Engine/Source/Editor/GameplayTasksEditor/Private/K2Node_LatentGameplayTaskCall.cpp`);
when you create a task from C++ you call `ReadyForActivation()` yourself
(`Engine/Source/Runtime/GameplayTasks/Classes/GameplayTask.h`). Never fire an output delegate
from the factory. Gate every broadcast on `ShouldBroadcastAbilityTaskDelegates()`.

Template: `templates/LatentAbilityTask.h/.cpp` and `templates/InstantNode.h`.

## The six rules, in the order they bite

### 1. One job, named like an API

`<Verb><Object>[<Qualifier>]`, verb-first, imperative, **no subject** (the system is the
subject). `Get*`/`Set*`, `Is*`/`Has*`/`Can*`, `WaitFor*`. Acronyms stay UPPERCASE: `HUD`,
`UI`, `AI`, `ID`.

| Bad | Why | Good |
|---|---|---|
| `HeGetsParried` | Narration, subject, tense | `ApplyStagger` |
| `LetTheOtherManAnswer` | A sentence invented for one call site | `WaitForIncomingHit` |
| `ResolveHeavyStrikeVsParry` | Two verbs' names inside one symbol | `ResolveHit` |
| `DoStuffAfterDodge` | No job | `OpenRiposte` |
| `GetHudScale` | Lower-cased acronym | `GetHUDScale` |

Domain *terminology* is welcome and exact: `Parry`, `Riposte`, `Stagger`, `Poise`. Domain
*narration* is not. A term is a noun the genre already owns; narration is a clause you
invented.

**Size vs name.** "Could an onlooker say it happened?" is a good test of how *big* a node is
("he parried it" - yes; "he lines up the preconditions" - no). It is a terrible test of what a
node is *called*. Use it to size, never to name.

### 2. Inputs only what it cannot derive

```cpp
// Bad: the node already knows the attacker (it is the ability's avatar), and the
// enum is converted straight back into an actor inside.
UFUNCTION(BlueprintCallable) void ApplyStaggerTo(AActor* Attacker, EStrikeSide Side);

// Good: the one thing it cannot know.
UFUNCTION(BlueprintCallable, Category = "Melee|Parry")
void ApplyStagger(AActor* Target);
```

An enum that the node maps to an actor is a pointer with hardcoded destinations. Pass the thing.

### 3. Outputs only what it computes

Never emit something the node merely held or observed - that is a getter somewhere else. A pin
that is null half the time means the node does not know what it returned; split it into exec
outputs so the value only exists on the path where it is meaningful.

### 4. Hand the result on a wire, never leave it in a field

If node A writes `bParried` into a member and node B reads it, the graph shows two unconnected
nodes and hides the dependency. Nothing stops an author putting B before A. **The wire is the
state**: being at the wrong point in a sequence stops being possible rather than being caught.

### 5. Fork only where the author has two different things to draw

```cpp
UENUM(BlueprintType)
enum class EParryOutcome : uint8 { Parried, Blocked };

// One exec output per enumerator, so no Switch node and no Break node in the graph.
UFUNCTION(BlueprintCallable, Category = "Melee|Parry", meta = (ExpandEnumAsExecs = "Outcome"))
void ResolveIncomingHit(const FGameplayEventData& Hit, EParryOutcome& Outcome);
```

`ExpandEnumAsExecs` ("Also works for bools") is documented in
`Engine/Source/Runtime/CoreUObject/Public/UObject/ObjectMacros.h`. A latent task forks by
having one `BlueprintAssignable` delegate per outcome.

Do not fork where nothing differs. A "did not happen" output on a node called `CommitParry` is
not a fork; it is a failure the node should have refused before activation.

### 6. Nothing unsafe may be left unconnected

Prefer a shape the Blueprint compiler rejects over a runtime guard.

- **`UPARAM(Required)`** sets `CPF_RequiredParm` ("Parameter must be linked explicitly in
  blueprint. Leaving the parameter out results in a compile error",
  `Engine/Source/Runtime/CoreUObject/Public/UObject/ObjectMacros.h`). The call-function node
  reports `Pin ... must be linked to another node` (`ValidateRequiredPins` in
  `Engine/Source/Editor/BlueprintGraph/Private/K2Node_CallFunction.cpp`). The engine uses it
  itself, for example `SetMouseCursorWidget` in
  `Engine/Source/Runtime/Engine/Classes/GameFramework/PlayerController.h`.
- **`UPARAM(ref)`** makes a by-reference input. Unless the function has `AutoCreateRefTerm`
  meta for it, an unwired by-ref input fails validation with *"must have an input wired into it
  ("by ref" params expect a valid input to operate on)"* (`IsPinDefaultValid` in
  `Engine/Source/Editor/BlueprintGraph/Private/EdGraphSchema_K2.cpp`, reported as a compile error
  through `Engine/Source/Editor/KismetCompiler/Private/KismetCompilerMisc.cpp`). Use it to make
  a token that must travel on a wire - a riposte opening that must be spent or closed - undrawable
  when leaked.

Both checks run on the function-call node. Verify the behaviour on your own engine version
before relying on it for a latent task's factory pins.

Template: `templates/RequiredPinNode.h`.

## Extension seams: `BlueprintNativeEvent`, not `protected virtual`

A Blueprint child cannot override a C++ `virtual`. If a subclass is meant to change a behaviour,
make it a `BlueprintNativeEvent`: the native body is written as `<Name>_Implementation`, and the
generated thunk calls the Blueprint override when there is one (see the `BlueprintNativeEvent`
entry in `Engine/Source/Runtime/CoreUObject/Public/UObject/ObjectMacros.h`). Use
`BlueprintImplementableEvent` only when there is no sensible native default.

Use a seam for a genuine per-verb difference in *computation* (how a Heavy Strike scales poise
damage). Do not use one to let a Blueprint re-implement the sequence the graph already draws.

Template: `templates/NativeEventSeam.h`.

## The C++ instinct, one layer too far out

| Good C++ instinct | At the Blueprint boundary | Do instead |
|---|---|---|
| Bundle params into `FStrikeParams` | Opaque pin needing Make/Break; parts invisible; struct duplicated per call site | Flat pins, authored as literals |
| Return `FStrikeResult`, let the caller switch | Author draws Break + Branch, then does arithmetic | Exec outputs; compute the final number in C++ |
| Overloads and default arguments | One node with pins nobody can interpret | Separate nodes, one job each |
| Encapsulate state in members | The next node reads it behind the author's back | State on the wire |
| `protected virtual` | Unreachable from Blueprint | `BlueprintNativeEvent` |
| Rename freely | Orphans every graph that calls it | Land C++ and graph edits together, or add a CoreRedirect |

## The same value, expressed twice

| Value | In C++ | At the Blueprint boundary |
|---|---|---|
| Invariant | `check` / `ensure` | A pin shape the compiler rejects |
| Encapsulation | `private` / `protected` | Not a `UFUNCTION` |
| RAII | Destructor | The task's `OnDestroy`, or a token travelling on a wire |
| Type safety | The type system | The node's shape and its pin types |
| Naming | Internal, renameable | Public API, effectively forever |

## Naming is forever (almost)

Renaming a `UFUNCTION` that graphs call leaves every calling node orphaned; the C++ build is
green and the Blueprints fail only when compiled or run. Your options:

1. Land the C++ rename and the re-authored graphs in the same change, then compile every
   affected Blueprint.
2. Add a redirect so existing nodes rebind:
   `+FunctionRedirects=(OldName="MeleeAbility.ResolveHeavyStrikeVsParry",NewName="MeleeAbility.ResolveHit")`
   under `[CoreRedirects]` in your project's `DefaultEngine.ini`. The engine ships many of these in
   `Engine/Config/BaseEngine.ini`.

Renaming only a *parameter*, or adding `DisplayName` meta, does not orphan the node; removing a
parameter that a graph had typed a literal into leaves a warning, not an error.

## Checklist for a new node

- [ ] Latent only if time passes; named `WaitFor<What>`.
- [ ] Name is `<Verb><Object>[<Qualifier>]`, no narration, acronyms uppercase.
- [ ] No input the node could derive (avatar, owning ASC, world).
- [ ] No enum input that becomes an actor.
- [ ] No struct input or output a designer would have to Make/Break.
- [ ] Exec outputs exactly where the author draws different things; no half-null pins.
- [ ] No member written by one node and read by another.
- [ ] Anything that must be connected is `UPARAM(Required)` or `UPARAM(ref)`.
- [ ] No arithmetic left for the graph to do.
- [ ] `Category` set, tooltip comment written - it is documentation for designers.
