# Merging lanes, gating, and keeping lanes busy

## Branch shape

- Each lane works on its own branch in its own worktree.
- Lanes merge into a **round** (integration) branch. Main moves only after a full gate on the
  round.
- A lane pushes after **each finished item, at a clean commit**. The integrator merges what has
  arrived; nobody waits on a lane mid-item.
- The developer calls the cut for a playtest build. Lanes keep landing work until then; do not
  impose cut-off times on them, and keep the last green package as the fallback.

## Merging generated content

Baked levels, generated meshes and anything a script writes from inputs are **outputs**.

- On a conflict in a generated asset, take either side and **re-run the generator** on the
  merged inputs. Never hand-merge a binary asset, and never trust either side as-is: each was
  generated from inputs the merge has changed.
- After a merge that took another branch's level, **re-run the derived setup steps** that
  write into it. In the running example, the arena bake replaced the persistent level and the
  merge took the other lane's copy, which dropped the always-loaded review sublevel's
  streaming entry. The review cameras silently vanished from every arena until the review
  setup script was re-run.
- Merge, then rebuild, re-import, re-bake, then run the bake check and the touched tests, in
  that order, in one place (the integrating tree).
- A level edited by two lanes is a conflict waiting to happen. One lane owns it at a time.

## Gates

| When | Gate |
|---|---|
| A content round (PCG graph, art import, dressing pass) | Editor build only if C++ changed; the tests covering changed C++; the bake check; a few captures |
| A gameplay change | Its own tests, written to fail without the change |
| A merge batch into the round | Bake check; cook and a short packaged smoke run; build if C++ changed |
| Before main moves | Non-adaptive-unity build, the full suite by name, data validation, cook, packaged smoke |

**Why light gating for content:** a full suite, cook and packaged smoke per art round doubled
round-trip time and caught nothing that the bake check and a capture did not. Visual fixes are
checked with a before/after capture, not a new regression test.

**Why the full gate before main:** an incremental build can exclude the very file that
collides in a unity build, so only a non-adaptive or clean build proves the tree builds from
scratch. A cook finds the Blueprint in an error state that the editor loads happily.

**Paperwork may batch, never drop.** Wiki pages, issue comments and tool notes can wait to the
end of a block if something tracks what is owed: a list every commit prints, and a pre-push
hook that refuses main while it is non-empty. Every brief ends with a checklist reported item
by item as DONE or NOT DONE.

## The overnight heartbeat

When the developer is away and lanes should keep working, a scheduled check (an hourly loop or
cron in the orchestrating session) keeps them busy:

1. For each lane: is its agent running, finished or stuck? Read its last report, not its
   context.
2. A finished lane with a clean pushed commit: merge it into the round (or queue the merge for
   the integrator), then start the **next item from that lane's queue with a fresh agent and
   a tight brief**. Do not resume the finished agent.
3. A stuck lane (no commit, no log progress for a long stretch): record what it was doing and
   stop it by its PID.
4. Respect holds: never start work the developer said to wait for, even on an idle lane.
5. Write one line per lane to one log the developer reads in the morning: what landed (SHA), what
   is running, what is blocked and why.

Check machine load before starting another lane: the editor cap applies overnight too.

## Retire long-context agents

An agent left running for hours grows to hundreds of thousands of tokens, and every turn
re-reads all of it. Brief agents for a bounded list (a few items), have each wrap at a clean
pushed commit with a report of where it stopped, and start the next item with a fresh agent
built from that report. Gate and packaging runs go to a small, cheap agent per batch, not a
resident one.
