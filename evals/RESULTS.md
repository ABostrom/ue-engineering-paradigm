# Eval results

**Skill version:** 0.1.0 **Model under test:** Claude Sonnet (fresh subagent per run)
**Method:** each task's *Prompt* was given to a fresh agent twice: once **without** the skill
(told to answer from its own knowledge and read no files) and once **with** it (told to read
`SKILL.md` and whichever references it found relevant). Answers capped at 600-700 words. Each
answer was scored against the task's rubric: one point per "must" item (half for partial), and
every "must not" violation counted separately.

## Summary

| Task | Baseline (must) | Baseline violations | With skill (must) | With-skill violations |
|---|---|---|---|---|
| 01 Design a Parry verb | 2 / 5 | 2 | **5 / 5** | 0 |
| 02 Review an enum-plus-switch ability | 1.5 / 4 | 1 | **4 / 4** | 0 |
| 03 Enemy type needs code | 6.5 / 7 | 0 | **7 / 7** | 0 |
| 04 God-class GameMode | 5 / 8 | 0 | **7 / 8** | 0 |
| 05 BlueprintCallable with struct params | 3.5 / 6 | 0 | **6 / 6** | 0 |
| 06 "Fallback when null" PR | 4 / 5 | 0 | **5 / 5** | 0 |
| 07 Replicated value never arrives | 2.5 / 4 | 0 | **4 / 4** | 0 |
| **Total** | **25 / 39 (64%)** | **3** | **38 / 39 (97%)** | **0** |

The skill wins on every task. The gap is largest where the paradigm is **opinionated**
(01, 02, 05): an experienced engineer's default answers there are reasonable C++ designs that
fail the "next verb with no programmer" test. It is smallest where good general Unreal practice
already gets you most of the way (03, 06).

## What the baselines did (RED)

These are the failures the skill was written against.

- **01 Parry.** Put the flow in a C++ `ActivateAbility` ("commit cost, apply the block effect,
  play the guard montage, and run the tasks below"); the Blueprint child only "assigns the
  definition asset, the montage, and the input tag" - an empty wrapper. Modelled the perfect
  parry as an `EHitOutcome::PerfectParried` enum branch plus a tier array. Resolution took an
  `FHitContext` struct.
- **02 Enum review.** Correctly requested changes and called the enum "a hidden class hierarchy",
  but proposed "one UGA_MeleeAttack with a data asset or an instanced-struct list of steps (play
  montage, wait for release, require airborne, hit N times, recover)" - the same trap, configured.
  No mention of one Blueprint graph per attack, or of what deleting/renaming the enum does to
  existing Blueprints.
- **03 Enemy types.** Strong: primary data asset per enemy, `TSoftClassPtr`, Asset Manager scan,
  `IsDataValid`, faction tags, per-asset files. Only the acceptance test was implicit.
- **04 God class.** Strong on GameState/PlayerState ownership, events instead of ticks, and the
  cheat manager. Missed that runtime-added components must replicate for clients to reference
  them, did not subtract before extracting, and did not check for an existing home.
- **05 Struct API.** Moved the 0.5 into C++ and removed the derivable attacker, but replaced the
  result struct with "an enum outcome ... so the graph can use a Switch node", kept a verb name in
  the new symbol (`ResolveHeavyStrike`), and did not mention that renaming orphans the calling graph.
- **06 Fallback PR.** Strong rejection with validation at the edge; did not distinguish authored
  data from untrusted runtime input.
- **07 Replication.** Diagnosed the archetype baseline correctly, but recommended a sentinel
  default or replicating the mode id and re-deriving on clients, not `bOutWantsFullInitState` or
  spawning at runtime; no word on why tests miss it.

## Changes made after the baseline (GREEN)

- Added **"What experienced engineers still get wrong here"** to `SKILL.md`: the empty wrapper
  over a C++ `ActivateAbility`, the step list in data, the outcome enum the graph switches on,
  the perfect-variant flag - and the counter-case that central hit resolution in C++ is correct.
- `node-contract.md` gained `ExpandEnumAsExecs` as the way to give a function exec outputs, and
  `UPARAM(Required)` beside `UPARAM(ref)` (both verified in engine source).
- Red flags for clients reading the GameMode, unreplicated runtime components and renames.

## With-skill answers (GREEN), highlights

- **01** drew the graph (`WaitForIncomingHit` with `Parried`/`Blocked`/`Expired` exec outputs),
  kept `ActivateAbility` out of C++, put tunables on `BP_GA_Parry`, made Perfect Parry
  `BP_GA_PerfectParry`, and explicitly rejected a step list and an outcome enum.
- **02** requested changes, named the falsifiable test, and gave a per-verb conversion order
  ending with "build Spin as a pure Blueprint child - the pass condition is that it needs zero new
  C++", plus the enum-deletion and resave hazards.
- **05** produced `ResolveHit(Target, BaseDamage, Outcome, DamageToApply)` with
  `ExpandEnumAsExecs`, `UPARAM(Required)` on the target, and the rename/CoreRedirect warning.
- **07** gave the `OnSerializeNewActor` / `bOutWantsFullInitState` fix, spawn-at-runtime as the
  alternative, the test-blindness note, and warned against a client-side second store.

## Refactor pass

The with-skill Parry answer said to validate the Blueprint child's CDO, which contradicted the
skill's blanket "return `NotValidated` for the CDO". The answer was right for abilities, whose
tunables live on the class default. `traps.md` #4 and `data-assets-and-validation.md` now
distinguish **content on instances** (skip the CDO) from **content on the class default**
(validate it, skip abstract bases).

## Residual gaps

- **04:** neither run said "check for an existing home before inventing a manager", although the
  decision procedure's first step says it. Worth a dedicated task if it recurs.
- One with-skill answer added advice that is not in the skill ("per-commit gating costs more than
  it saves"). Harmless here, but a reminder that agents blend the skill with their own priors.

## Re-running

Use `evals/tasks/*.md`. Give the *Prompt* section only; keep the rubric from the agent. Run
baseline and with-skill on fresh agents, score against the rubric, and add a dated section here.
