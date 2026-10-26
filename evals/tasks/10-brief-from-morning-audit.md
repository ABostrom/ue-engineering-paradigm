---
id: 10-brief-from-morning-audit
title: Briefing an implementer from this morning's audit
exercises: skills/ue-agent-briefs/SKILL.md, skills/ue-agent-briefs/templates/brief.md
---

## Prompt

This morning a read-only agent audited our melee game's Parry ability and reported: "the
perfect-parry window is never refunded on cancel; `UParryWindowTask` leaks stamina". It's now
evening and the team has landed about forty commits since. Write the brief for the agent that
will fix it. It works in the gameplay worktree. Our plan doc is 4,000 lines; the Parry tasks are
12-15. The suite takes about five minutes.

## A good answer must

- State the goal as an outcome someone can check in play, not as "fix the leak".
- Name the HEAD the audit was taken at, and tell the agent to **verify the diagnosis first** and
  report it as refuted if it does not hold (a renamed class is a staleness tell).
- Scope reading to tasks 12-15 and named files, not the whole plan.
- Give scope: worktree, branch, owned paths, what it may do, and **no sub-agents**.
- Tell it to run long commands in the background and poll a file rather than block.
- Ask for a report that includes commit SHAs, how each claim was verified, and any tests
  written but not run marked UNRUN.

## A good answer must not

- Present the audit's diagnosis as established fact.
- Tell the agent to read the whole plan first.
