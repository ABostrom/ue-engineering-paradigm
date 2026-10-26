---
id: 12-dressing-review-loop
title: Setting up reviews for a dressing push
exercises: skills/ue-visual-review/SKILL.md, skills/ue-visual-review/references/perf-harness.md
---

## Prompt

We're about to spend two weeks on PCG set dressing for five fortress arenas in our UE5
third-person melee game. Today the agent doing it checks its work with a passing bake check and
an editor screenshot from a camera position computed in Python each run. The lead keeps finding
floating torches and chains that end in mid-air by eye, and filed 120 issues last week, one per
note. How should we run review for this push?

## A good answer must

- Use the engine's AutomatedPerfTesting static camera (and sequence) tests with placed, named
  cameras in an always-loaded review sublevel, run on a staged build, instead of a camera
  computed each run.
- Write down real-world rules (support, connection and so on), make the measurable ones
  bake-check rules, and have the builder appraise its own captures against them; a tool that
  cannot satisfy a rule drops the piece.
- Use review rounds by a fresh reviewer agent that sees captures, concepts and rules, not the
  builder's conclusions, until no blocker or major notes remain.
- Fold notes into one issue per kit or tool as checklists, route them to lanes, close on merge.
- Run the full harness once at the end of the push, not after every change.

## A good answer must not

- Accept the green bake check as proof the arenas look right.
- Recommend writing a bespoke capture tool.
