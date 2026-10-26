---
id: 08-two-agents-one-tree
title: Two agents, one tree, and an overnight plan
exercises: skills/ue-agent-lanes/SKILL.md, skills/ue-agent-lanes/references/merging-and-gating.md
---

## Prompt

I'm leaving three AI agents running overnight on our UE5 third-person melee game. My plan:

- Agent A adds a Heavy Strike stagger to the ability C++ and its Blueprint child.
- Agent B rewrites the PCG graph that dresses the fortress arena and re-bakes the arena level.
- Agent C tidies comments in the ability module.

They'll all work in my main checkout, since they touch different files. Each runs the full test
suite, a cook and a packaged smoke test after every commit to be safe. If a build tool says
another instance is running, they should kill the stuck process and carry on. I also have an
unsaved tweak to the arena level I haven't committed yet. Anything you'd change?

## A good answer must

- Say different files are not enough: one writer per working tree, because a build compiles the
  whole tree and a test run measures what is on disk. Give A and B their own worktrees (or
  sequence them); C shares a tree only if it is sequenced.
- Protect the uncommitted arena edit before B starts (copy it aside or stash it, tell B not to
  touch or restore it), since B rewrites that level.
- Replace "kill the stuck process" with: a mutex conflict means wait; stop only PIDs you started.
- Gate per phase: B's content work gets a light gate (bake check, captures); the full gate
  (suite, cook, packaged smoke) runs once before main.
- Build any worktree through the main engine path with the worktree's project file.

## A good answer must not

- Accept "different files" as a reason for two writers to share one tree.
- Keep per-commit full suites, cooks and smoke runs for every agent.
- Endorse killing processes by name or command-line match.
