# Brief: <one-line outcome>

<!-- Fill every section. Delete the comments. A section you cannot fill is a question for
     the owner, not something to leave for the agent to guess. -->

## Goal
<!-- The outcome someone can check: what the owner does in play, sees in a capture, or finds
     in the tree. Not the work, the result. -->
After this, in a two-player session, Heavy Strike staggers an enemy that is parrying, and the
stagger reads clearly from the third-person camera.

## State
- Starts from `<branch>` at `<sha>`.
- Earlier passes: <what changed or was removed, and what is deliberately out of scope>.
- Diagnosis so far, as of `<sha>`: <the claim>. **Verify it before acting on it, and report
  it as refuted if it does not hold.**

## Read
- `<wiki page or doc>` (the subsystem page) before searching code.
- `<plan path>`, tasks <n>-<m> only.
- `<file>`, `<file>`.

## Scope
- Folder `<worktree>`, branch `<branch>`, editor port `<port>`.
- Owns: `<paths>`. Does not touch: `<paths>`.
- These files are the owner's uncommitted work: `<paths>`. Do not edit, save over or restore
  them.
- May: build / open the editor / run tests / commit. May not: push / merge / touch main.
- Build with `<the main engine path's Build.bat> <Target> Win64 Development
  -Project="<worktree>/<project>.uproject" -WaitMutex`.
- **Do not spawn sub-agents or workflows. Work serially. One heavy process at a time.**
- **Never block on a call longer than about two minutes**: run builds, suites and cooks in the
  background with output to a file and poll it with `tail` or `grep -c`. Grep finished logs;
  never read one whole.
- Stop only processes you started, by PID.
- No whole-screen captures; no foreground or cursor-moving work unless told the machine is
  free.
- Gate: <the light content gate | the phase gate>, per `<where the gate policy lives>`.
- If the plan is wrong, correct course and report it.

## Report back
- Commits (SHA and one line each), pushed to `<branch>`.
- What was verified and how: the command, the result line. Tests written but not run: **UNRUN**.
- Diagnoses refuted, and what you found instead.
- Anything left out, and why.
- What the owner should look at, specifically ("open the arena, stand by the north gate, Heavy
  Strike the parrying guard").
- Owed paperwork, each DONE or NOT DONE: <wiki pages, issue comments, tool notes>.
