---
id: 07-replicated-value-never-arrives
title: A replicated value that never arrives
exercises: references/traps.md
---

## Prompt

Our UE5 listen-server arena game has an `AArenaBoundary` actor **placed in the level**.
At match start the server reconfigures it from the chosen mode's data asset:
`Boundary->Radius = Mode->ArenaRadius;` (`Radius` is `UPROPERTY(ReplicatedUsing=OnRep_Radius)`,
and `OnRep_Radius` rebuilds the collision and the visual ring).

The map was authored with the boundary's instance `Radius` set to 1200. The Blueprint
`BP_ArenaBoundary`'s default `Radius` is 2000. Modes: "Pit" (800), "Skirmish" (1500)
and "Standard" (2000).

Bug reports: in Standard matches, clients see a 1200 ring while the server uses 2000.
Pit and Skirmish are fine. `ForceNetUpdate()` after setting it didn't help. What's going
on, and how do you fix it properly?

## A good answer must

- Explain that **replication sends differences from the archetype/CDO baseline**, not
  from what the client holds: in Standard the runtime value (2000) **equals the CDO**,
  so it is never sent, and the client keeps its map-loaded 1200.
- Explain why Pit and Skirmish work (their values differ from the CDO).
- Give a real fix: for example override `OnSerializeNewActor` and set
  `bOutWantsFullInitState = true`, or spawn the boundary at runtime instead of
  reconfiguring a placed actor, or make the map instance match the CDO.
- Note that a test whose fake network copies every property is blind to this.

## A good answer must not

- Blame relevancy, net priority or OnRep timing as the root cause.
- Recommend a multicast RPC or a per-tick resend as the fix.
- Stop at `ForceNetUpdate` / `MARK_PROPERTY_DIRTY`.
