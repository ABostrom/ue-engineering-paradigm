# Composition, god classes, and one owner per fact

## Thin by composition

The GameMode, the PlayerController and the Character are **doors and composition roots**, not
places where jobs live. A new responsibility goes into a component or subsystem the class owns
or finds - never into the class body.

**Why the rule keeps losing:** a standing rule only fires when something prompts a lookup. Each
change is framed as "fix this bug", the code already lives in the controller, so the fix goes in
the controller. "Fix the cancel" never prompts "does this belong here?". Eight small fixes later
the file is 3,000 lines. **Ask "does this belong here?" on every edit to a big class, not only
on refactors.** Better still, give the rule a test that fails when a job is in the wrong place
(for example: every verb's preview comes from the ability, not the controller).

### What stays

| Class | Keeps | Sheds to |
|---|---|---|
| GameMode (server only) | Login/seating, spawning the match, the commit door for authoritative requests, the ruleset reference | Match phases, scoring rules, respawn policy, loot, tutorial logic |
| GameState | Replicated match facts clients read | Logic (it publishes; it does not decide) |
| PlayerController | RPC entry points, possession, the current selection, cancel | Input mapping, camera, picking/targeting, HUD/widget management, cheats |
| Character / Pawn | Movement, the ability system component, the mesh | Per-feature behaviour (as components or abilities) |

### Where each job goes

| Job | Home |
|---|---|
| A behaviour attached to one actor | `UActorComponent` on that actor |
| A world-wide service (registry, spatial query, spawner) | `UWorldSubsystem` |
| Per-local-player UI / input state | `ULocalPlayerSubsystem` or a component on the controller |
| A match phase's work | The phase itself (below) |
| Console commands and test cheats | `UCheatManager` subclass or `UCheatManagerExtension` (`Engine/Source/Runtime/Engine/Classes/GameFramework/CheatManager.h`) |
| A fact clients must read | One replicated property on GameState / PlayerState / a replicated component |

## Phases do their phase's work

A match flow written as `switch (Phase)` in the GameMode's `Tick` is a god class in waiting.
Make each phase an object that **does the work of its phase**:

- **Loading/Warmup** holds the start gate (waits for players and content).
- **Starting** lays the match out (spawns teams, seats players).
- **Playing** runs the round loop.
- **Results** ends the match and shows the outcome.

Lyra's sample does this with GAS abilities as phases, managed by a game-phase subsystem; a
phase ability activates, does its work, and ends when the next phase starts. Whether you use
abilities or plain objects, the shape is what matters: **the phase owns its work**, the mode
asset names which phase classes run, and the GameMode only starts the first one. A new phase is
a new class or Blueprint added in data, not a new `case`.

Replace "check every tick whether the round is over" with an **event from the owner of the
fact**: whoever records a death asks the rules whether that ended the round.

## One owner per fact

Every fact has exactly one owner. Everything else either **reads it** (through a published,
replicated mirror the owner writes) or **derives it** (a query computed from the owner's data).
Never a second store.

| Pattern | Good / bad |
|---|---|
| GameState publishes `Kills` per PlayerState; HUD reads it | Published mirror - good |
| `GetScoreLeader()` computes from PlayerStates when asked | Derived query - good |
| GameMode caches `TMap<APlayerState*, int32> Kills`; GameState also has kills; HUD reads the GameMode | Second store - bad, and broken on clients (the GameMode does not exist there) |
| A roster asset listing which ability Blueprints ship, alongside the ability set that grants them | Second store - they drift |

**The tell:** a comment saying two things "must agree". They will drift, silently. Delete one,
or derive it. Often the fact is already answered by something structural - "which Blueprint
wraps this native class" is answered by the asset registry from parentage; no list required.

## Do not invent a system that has a home

Before building a registry, map, list, manager or catalogue, **find what already holds that
fact**. If you cannot name the system a new piece belongs to, either you missed the existing
home, or it is a real gap - both are a conversation, not a quiet invention.

- Prefer **deriving** over **recording**. A list is a second copy of a truth.
- Do not widen a scoped change into "while I'm here, add a mechanism". The mechanism usually
  already exists.
- A finding is a finding. It is not a licence to build.

## Ask whether a dependency exists, not what world you are in

```cpp
// Bad: a list of world types the caller *believes* the subsystem supports. It must agree
// with the subsystem's DoesSupportWorldType - and it will drift.
if (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) { ... }

// Good: ask the dependency.
if (UArenaRegistry* Registry = World->GetSubsystem<UArenaRegistry>()) { Registry->Register(this); }
```

`UWorldSubsystem::DoesSupportWorldType` (`Engine/Source/Runtime/Engine/Public/Subsystems/WorldSubsystem.h`)
is the one place world type is decided for a subsystem. A caller's copy of that list fails two
ways: a world type on the list with no subsystem crashes; a world type off the list with a
subsystem **silently does nothing** - the worse failure, in editor previews and tools.

World type *is* a legitimate question for a genuine behaviour difference (snapping a placed
actor to a grid in the editor but not during play) - not for a dependency lookup.

## Readiness: "has this finished spawning?"

When several components must be ready before an actor is usable (data assigned, abilities
granted, team set, visuals built), a single "spawned" event is not enough - it fires when *one*
system has done its part. Use a readiness ladder with a **gate**: each component can say "not
yet", and anyone can wait for a state without knowing who provides it.

The engine has one: `UGameFrameworkComponentManager` with `IGameFrameworkInitStateInterface`
(`CanChangeInitState`, `RegisterAndCallForActorInitState`, in
`Engine/Plugins/Runtime/ModularGameplay/Source/ModularGameplay/Public/Components/GameFrameworkComponentManager.h`
and `GameFrameworkInitStateInterface.h` beside it). If you roll your own enum ladder, keep the
gate - a ladder without a gate is just a counter.

## Server-only classes, replicated pieces

- The **GameMode exists only on the server**. Anything a client needs - HUD values, phase,
  scores - must be on GameState, PlayerState or a replicated component. A HUD that reads the
  GameMode works for the host and fails for every joining client.
- A component the server **adds at runtime** (composed from a mode asset, granted by a phase)
  must replicate - `SetIsReplicatedByDefault(true)` - **even if it has no replicated
  properties**, if anything ever references it over the network (for example as an ability's
  source object). See `traps.md`.
- On a listen server the host is not a network path: testing as the host proves nothing about
  replication. Test with a real second client after every replication-relevant step.

## Decomposition order

1. **Subtract first.** Delete dead code, duplicated blocks, guards for impossible paths, and
   history-narrating comments. Often 30-40% of a god class. Nothing moves yet.
2. **One answer per question.** Where two code paths answer the same question differently
   (two "acting characters", a per-tick retry and an event), pick one.
3. **Move facts to their one owner** (replicated state to GameState/PlayerState) and rebind
   readers.
4. **Extract jobs one at a time** into components/subsystems/phases, cheats to the cheat manager.
5. **Re-scale and add** only after the shape is right.

Keep the game playable after every commit; source control is the safety net, not a big-bang
branch. When moving a value from a Blueprint default onto a new component, resave the Blueprints
(`traps.md`).
