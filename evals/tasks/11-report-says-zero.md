---
id: 11-report-says-zero
title: A report that says the problem is gone
exercises: skills/ue-agent-briefs/references/reading-reports.md
---

## Prompt

An agent reports back on our fortress arena dressing work:

> Fixed the floating braziers: the bake check's `floating_props` rule went from 40 to 0. Also
> added 6 tests for the new brazier placement; they compile (the editor had the DLLs locked so
> I couldn't run the suite). Commit message: "Fix floating braziers and add placement tests".
> Done.

How do you read this, and what do you tell the owner?

## A good answer must

- Treat the drop to zero as unexplained until the denominator is checked: are braziers still
  being placed at all, and is the rule still looking at them?
- Report the six tests as **UNRUN**, not done, and expect the first run to find something.
- Check what landed against the tree (`git show --stat`, the diff), not the commit message.
- Ask for (or take) a capture of the area, because a bake rule count does not prove it looks
  right.
- Not call it "fixed" to the owner.

## A good answer must not

- Summarise the task as done.
- Ask the agent to add more tests as the remedy.
