---
id: 04-god-class-gamemode
title: Decompose a god-class GameMode
exercises: references/composition-and-god-classes.md, references/traps.md
---

## Prompt

Our multiplayer (listen-server) melee arena game's `AArenaGameMode` is 3,000 lines. It
runs the match flow (warmup, rounds, sudden death, results screen), scoring, respawns,
loot drops, a tutorial mode, an "is the round over" check every tick, a cached
`TMap<APlayerState*, int32> Kills` that the HUD also reads through a GameMode getter,
and about 30 console commands for testing. The player controller is similar at 2,500
lines. Plan the decomposition. Where does each job go, and in what order would you do it?

## A good answer must

- Split jobs into **components and subsystems by responsibility**, keeping the GameMode
  and controller **thin** (the classes hold the doors and the composition, not the jobs).
- Make **match phases own their phase's work** (for example GAS phase abilities or a
  phase subsystem, as in Lyra), rather than a giant state switch.
- Note that the **GameMode exists only on the server**, so anything clients read (the
  HUD's kills) must be **one replicated store** on GameState/PlayerState, not a
  GameMode getter and not a second cached copy.
- Note that **components added at runtime must replicate** (`SetIsReplicatedByDefault`)
  if a client ever needs to reference them, even with no replicated properties.
- Replace the per-tick poll with an **event** from the owner of the fact.
- Move console commands to a **CheatManager** (or a cheat extension), out of gameplay classes.
- Order it as **subtract first** (dead code, duplicates), then extract one job at a
  time, keeping the game playable and testing with a real client after each step.
- Check for an **existing home** before inventing a new manager/registry.

## A good answer must not

- Propose a big-bang rewrite, or one `XManager` singleton per noun with duplicated state.
- Leave the HUD reading a GameMode getter.
- Add components to the GameMode that clients need to see.
