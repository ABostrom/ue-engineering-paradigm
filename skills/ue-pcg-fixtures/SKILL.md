---
name: ue-pcg-fixtures
description: Use when testing or debugging procedural content in Unreal (PCG graphs, dressing tools, routers that lay pipes, cables or chains, spawners driven by mesh sockets and volumes) - turning a review note about a bad spot into a reproducible case, building hand-placed test levels for a PCG system, writing a functional test that asserts on a bake check, marking a known bug as expected to fail, or deciding what a hand-made mesh must carry to replace generated greybox. Also when a PCG fix is verified only on a whole level, a rule was loosened to turn a test green, a review note cannot be pointed at one exact spot, or the pipeline reads a generator's naming or offsets instead of what the mesh declares.
---

# Procedural content fixtures and the artist contract

**A review note about a procedural system becomes a fixture first.** Rebuild the spot by hand
as a small canvas, write down what the real world needs there, watch the system fail, then fix
the system until the canvas passes. A fix verified only on a whole level is verified on
whatever that level happened to contain.

## The canvas

One **level per system** (the cable router, the rain drainage, the torch and banner mounts, the
rope bridges in the running example's fortress arenas), holding a row of **canvases** spaced
far enough apart that nothing one lays can reach the next. Each canvas has an id,
`Fix_<System>_<Case>` (`Fix_Chains_GatehouseCorner`), and **every actor, camera, log line and
report carries it**, so a note can point at one exact joint.

A canvas holds:

| Actor | Name | Job |
|---|---|---|
| The pieces | under folder `<id>/Pieces` | Hand-placed kit pieces, tagged exactly as a spawner would tag them, standing on the kit's grid |
| Footprints | trigger boxes, tagged | The building's inside, or whatever region the system reads |
| A PCG volume | `<id>_Volume` | Runs the system's graph alone, generation on demand, over this canvas only |
| A functional test | `<id>` | An `AFunctionalTest` subclass bounded to that volume, holding the expectations |
| A label | `<id>_Label` | The id and the case in words, standing in front of the plot |
| Review cameras | `<id>_Front`, `_Left`, `_Right`, `_Close` | `AAutomatedPerfTestStaticCamera`s; `_Close` on the tricky joint (see `ue-visual-review`) |
| A joint marker | `<id>_Joint` | A target point at the exact spot a joint expectation is about |

Building it, the test's sequence and the traps: **references/canvas-levels.md**.

## The test asserts counts from the bake check

The test generates its volume, waits until the spawned instances stop changing, then runs the
same bake check the real levels use, scoped to the canvas, and holds the results to its
expectations:

- **Rules at zero**: named bake rules (floating, clipping, run ends off a fitting, keep-out
  intrusions) must count 0 on what the system laid. The canvas's own hand-placed pieces are
  reported, not held.
- **Service minimums**: at least N runs laid, at least this share connected, every hub fed.
- **Joint expectations**: a run passes within R of the marker, going the right way.
- Every failure is logged with the id and an example location.

**Expect what a real building needs, never what the system does today.** A canvas written to
match current output tests nothing.

## Known bugs: expected to fail, never loosened

- A canvas the system fails today carries an **expected-failure reason** naming the issue. Its
  failures are logged as one warning and the test passes, so the suite shows it without
  blocking. When it **starts passing**, it warns "passes now; clear its expected failure".
- **Never loosen a rule, raise a threshold or delete an expectation to go green.** The canvas
  exists because the rule is right and the system is wrong. Loosening it deletes the record of
  the bug and lets every level regress with it.
- Seventeen canvases all failing on their first run is a good first run: it is the bug list,
  each with a camera pointed at it.

## The contract is the interface

Fixtures test the system against **what the pieces declare**, which is also what lets
hand-made art replace generated greybox later. A PCG tool reads only the contract: form and
style tags, sockets and connectors, occupancy and keep-out volumes, declared lengths. Never a
generator's internal names, offsets or quirks. The contract, how an artist supplies it, and how
a missing part is derived at import: `ue-engineering-paradigm`
**references/content-pipeline-and-taxonomy.md**, section "The contract is the interface".

## Red flags

| You see | It means |
|---|---|
| A PCG fix verified by re-baking a whole level and looking | No reproducible case; the next change can undo it unseen |
| A test threshold changed in the same commit as the system | The rule moved to meet the system |
| An expected failure with no issue named | A bug nobody owns |
| A canvas whose expectations were written from the current output | A test of nothing |
| Fixture levels in a plugin that cooks, or one that may not reference every kit | Fixtures shipping, or failing validation |
| A tool that reads a mesh's name to find its sockets or size | Hand-made art will not slot in |
