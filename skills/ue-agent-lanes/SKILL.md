---
name: ue-agent-lanes
description: Use when running more than one AI agent on an Unreal project at once, or one agent unattended overnight - setting up a git worktree per lane, launching a headless editor or editor Python from the command line, building C++ from a second worktree, merging lane branches or generated levels, deciding what to gate before a merge or before main, or stopping a hung process. Also when a build fails on symbols the agent never touched, a test count moved with no test added or removed, an editor never exits after its log closes, the shared engine suddenly rebuilds from scratch, a saved level turns out to be the wrong map, or the owner's uncommitted work has gone missing.
---

# Running agent lanes on an Unreal project

**One writer per working tree. Everything else follows from protecting that.**

An Unreal project is one build graph, one asset registry and one machine. Two agents editing
different files still share all three: a build compiles the whole tree, a test run measures
whatever binaries are on disk, and an editor holds DLLs and the GPU. Lanes are how several
agents work at once without measuring each other's half-finished edits.

## A lane

A lane is **a worktree + a branch + its own editor endpoint + the paths it owns.** For the
running example, a third-person melee action game:

| Lane | Owns | Never touches |
|---|---|---|
| Gameplay | ability C++ and the Parry, Dodge and Heavy Strike Blueprint children | levels, materials |
| Arena | PCG graphs, arena levels, bake scripts | ability code, materials |
| Look | materials, lighting presets | PCG graphs |
| Art | DCC sources and their generator scripts (no editor) | anything under `Content/` |

Every brief names its folder, branch, editor port and owned paths. A shared asset (the main
arena level, a project-wide data asset) belongs to **one lane at a time**.

Parallel agents in **one** tree are for read-only work only: audits, sweeps, reconnaissance.
Two write passes are sequenced, or one gets its own worktree.

## Rules, and the failure behind each

| Rule | What went wrong without it |
|---|---|
| One writer per tree | A comment-only pass failed its build 99 times on another agent's half-finished refactor in files it never opened, and its test count was off by exactly the tests the other agent's uncommitted work deleted. Neither agent did anything wrong. |
| Build a worktree only through the **main engine path**, with the worktree's `.uproject` | A lane ran `Build.bat` through its worktree's junction to the shared engine. UnrealBuildTool saw the engine at a second path, judged it out of date, and deleted and rebuilt thousands of shared engine outputs. Every lane's builds stopped until a relink with all editors closed. |
| Editors launch with Live Coding off; builds use `-WaitMutex` | Live Coding's mutex is keyed on the shared editor executable, so one lane's editor blocks another's. A build waiting on the mutex is queued, not broken: never kill it. |
| Stop only the PIDs you started | A mutex conflict was "cleared" by killing every process whose command line matched the build tool. One was the IDE installing its engine plugin, eight minutes in: a re-download, a full antivirus rescan and half an hour of the owner's time. |
| Cap concurrent editors (two headless is a good start) and run a watchdog | Off-screen editors sharing one GPU hung after `Log file closed`, stuck in driver teardown and immune to `Stop-Process`, while still holding module DLLs, so no lane could link. |
| Protect the owner's uncommitted changes before an agent enters a tree | An agent's bake overwrote the owner's unsaved level edit, then "cleaned up" by restoring the file to HEAD. The only autosave was older than HEAD. |
| An agent never restores a file it did not change | Same incident. A dirty file the agent did not make is someone's work, or a fix (see `ue-engineering-paradigm` traps #9). |
| Restore LFS assets with `git checkout`, never `git show rev:path >` | That writes the 130-byte pointer over the asset, and `git diff` then calls it identical (`ue-engineering-paradigm` traps #10). |
| Merge lanes at commit boundaries; re-bake generated levels instead of hand-merging them | A baked level is an output. Both sides of a conflict are wrong for the merged inputs. |

Details: **references/headless-editor.md** (launching, quitting, saving the right level, the
watchdog) and **references/merging-and-gating.md** (protecting work, merging, gates, the
overnight heartbeat).

## Before an agent enters a tree

1. `git diff --name-only` (scoped to the project, not a full status on a huge repo). Anything
   listed that the agent was not sent to change is someone's work.
2. Copy those files aside (or `git stash push -- <paths>`), and write in the brief: *"these
   files are the owner's; do not touch, save over or restore them."*
3. Note the HEAD the agent starts from, so its report can be read against it.

## Gates

**Gate per phase, not per commit.** Content rounds (PCG graphs, art imports, dressing) gate
lightly: build only if C++ changed, only the tests that cover changed C++, the bake check,
a few captures. The **full gate** runs once before main moves: a non-adaptive-unity build,
the whole suite by name, data validation, a cook and a packaged smoke run.

A suite number from a tree another agent was writing in is not a gate. Re-run it once the
tree is quiet.

## The machine is someone's desk

Headless work (builds, `-nullrhi` suites, commandlets, editor Python) is fine any time.
Anything that opens a window, moves the OS cursor or captures the screen waits until the owner
says the machine is free. **Never capture the whole screen**: an agent that widened a capture
to recover a stuck editor photographed the owner's video call. If a window handle cannot be
resolved, stop.

## Red flags

| You see | It means |
|---|---|
| Build errors naming symbols the agent never touched | Another writer in the tree |
| A test count moved and nobody added or removed a test | The suite measured someone else's working copy |
| `Build.bat` or the editor launched through a worktree's `Engine` junction | The shared engine is about to rebuild |
| `Get-Process <name> \| Stop-Process`, or a kill matched on a command line | Someone else's work is about to die |
| A hand-resolved conflict in a `.umap` | A generated level that will not match its inputs |
| A brief with no owned paths, or two lanes both owning one level | Two writers waiting to happen |
| An agent "restoring to HEAD" a file it did not change | Lost work |
