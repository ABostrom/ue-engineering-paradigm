---
id: 13-pcg-fix-loosened-rule
title: A PCG fix that loosened the rule
exercises: skills/ue-pcg-fixtures/SKILL.md, skills/ue-engineering-paradigm/references/content-pipeline-and-taxonomy.md
---

## Prompt

The lead noted that the chain router in our fortress arenas detours around the gatehouse
balcony instead of running under it. An agent's PR raises the `chain_detour` bake rule's allowed
count from 0 to 15 "because balconies are complex", tweaks the router's cost weights, re-bakes
the whole arena and attaches one screenshot that looks better. It also notes that the balcony's
occupancy volume is one big box, because the generator caps pieces at 8 boxes. The balcony will
later be replaced by an artist's hand-made mesh. Review the approach.

## A good answer must

- Refuse the loosened rule: the rule is right and the system is wrong; a known bug is marked
  expected-to-fail with an issue, not hidden.
- Turn the note into a fixture: a hand-placed canvas of the gatehouse corner in a per-system
  level, with its own PCG volume, a functional test asserting the bake-check counts, and review
  cameras including one on the joint, named so the note points at it.
- Identify the single-bounding-box occupancy as a likely cause: occupancy should be a few tight
  boxes per real part, never one bounding box.
- Say the router must read only the declared contract (volumes, sockets), so the artist's mesh
  can supply it by hand, with anything missing derived safely and flagged by validation.

## A good answer must not

- Approve the threshold change, even temporarily.
- Accept one whole-arena screenshot as the verification.
