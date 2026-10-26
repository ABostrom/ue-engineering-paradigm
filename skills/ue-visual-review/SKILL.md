---
name: ue-visual-review
description: Use when an Unreal project's look, level dressing, PCG output or player-facing feature needs reviewing by a person or an agent - setting up repeatable review shots or flythroughs, capturing before/after images and frame costs, publishing captures for the developer, running art-direction review rounds, turning review notes into issues and lane work, or deciding when to run the review harness. Also when a green test suite or bake check is being offered as proof that something looks right, captures differ run to run, review notes are piling up as hundreds of issues, or the developer keeps finding floating, clipping or nonsensical placement by eye.
---

# The visual review loop

**A green suite proves the rules it has. It does not prove the place looks real or the move
reads well.** The developer finds those by eye, and every one they find is a review the loop
should have done first.

The loop: **fixed shots → captures from a real build → a review against written rules →
notes as issues → lanes fix them → one big re-capture at the end of the push.**

## Fixed shots, from the engine's own harness

Use the **AutomatedPerfTesting** plugin rather than a capture script of your own. Its static
camera test visits every `AAutomatedPerfTestStaticCamera` in the loaded world, records frame
and GPU time per camera and takes a screenshot; its sequence test plays a Level Sequence end to
end with an event per camera cut. Gauntlet drives both against a staged build. You get the same
shots every time, under each lighting preset, with their cost beside them.

- Put the cameras in an **always-loaded review sublevel** per level, so bakes and dressing
  passes never touch them and they never dirty a bake.
- Name each camera's **object**, not only its label: a packaged game has no actor labels, and
  the test names the screenshot and CSV after the actor's name or label.
- A flythrough per level doubles as an intro montage and shows what fixed shots miss.

Setup, commands and traps: **references/perf-harness.md**.

For gameplay features the equivalent is an **eyewitness pass**: drive the feature in a running
game and photograph it. One such pass after a fully green night found nine defects, every one
of which passed its board-state assertions: each piece was right on its own, and they disagreed
on screen, or the fact was right and its presentation unreadable.

## Review against written rules

"Does it look good?" gets a shrug. "Does every pipe end at a fitting?" gets a finding. Write the
real-world rules down and review against them: **references/realism-rules.md**.

- **Appraise your own output before the developer sees it.** A tool that "places pieces" and a
  green bake check proved nothing about floating props, clipping panels or cables running
  nowhere; the developer found all three by eye.
- **Measurable rules become bake-check rules** with counts. The rest are a checklist the
  reviewer reports against.
- **When a tool cannot satisfy a rule, it drops the piece** rather than placing nonsense.

## Art-direction rounds

Dressing and look work goes through rounds of review by a **fresh** art-director agent:

1. Implement a whole block of the build order (not one step).
2. Bake, capture the fixed shot set.
3. A fresh read-only agent reviews the captures against the concept art, the art direction and
   the realism rules. It gets **captures, concepts and rules only, never the builder's
   conclusions**: a builder grading its own output stops early.
4. Each note gets a severity (blocker, major, minor, nit).
5. Fix, and run the next round. Stop when a round returns no blocker or major notes. Report
   each round's counts.

Be nit-picky here and spend freely: set dressing is low-risk and is what sells the world.

## Notes become issues, issues become lanes

- One issue **per kit or per tool**, with the notes as a checklist, not one issue per note.
  Search open issues before filing. A day of review rounds that filed an issue per note produced
  over a hundred, and nothing closed them when the work merged.
- A note that is a systems bug becomes a **fixture** first (see `ue-pcg-fixtures`): rebuild the
  spot, watch it fail, then fix the system.
- Route each issue to the lane that owns the fix (`ue-agent-lanes`). Reference the issue in the
  commits; close it with the SHAs when every item has landed. Triage after each merge.

## Publish captures for the developer

Before/after pairs, the same camera side by side, on one page the developer can read **on a
phone**: one column at narrow widths, images full width, the camera name and the note it
answers under each pair, the frame-time change beside it. Commit the images (they are the
progress record); scratch directories vanish.

## When to run the harness

**Once, at the end of a push**: after the big merge, before the developer's review. Not after
every change: each run needs a cook and stage, and per-change runs slowed round trips without
finding more. Inside a push, a cheap editor viewport shot of the touched area is enough.

## Red flags

| You see | It means |
|---|---|
| "Suite green, bake check OK" as the proof for a dressing pass | Nobody has looked |
| A capture script that positions the camera from code each run | Shots that drift; use placed cameras |
| Review cameras in the persistent level that a bake rewrites | Cameras lost on the next bake or merge |
| The builder reviewing its own captures | A review that stops early |
| An issue per review note | A list that grows forever |
| A harness run after every commit | A cook per change for no new information |
| A whole-screen capture | Never; captures are window- or viewport-scoped (`ue-agent-lanes`) |
