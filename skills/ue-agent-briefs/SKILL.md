---
name: ue-agent-briefs
description: Use when handing Unreal project work to an AI subagent, or reading what one sent back - writing a brief, choosing its model, deciding what it reads, running a lookup or audit, or deciding whether to resume or restart an agent. Also when a report says done but tests never ran, a count dropped to zero, a commit message and the tree disagree, an audit from earlier in the day is being acted on, a session is burning its usage window faster than the work explains, an agent spawned agents of its own, or the owner corrected the same thing twice and the lesson needs a home.
---

# Briefing and trusting agents

**A brief is the only context the agent has, and its report is the only thing you see.**
Both handoffs drop detail. The brief decides what the agent can get right; reading the report
with suspicion decides what you pass on as true.

## The brief

Five parts, every time. The full template is **templates/brief.md**.

1. **Goal, as an outcome someone can check**: what the owner does in play, sees in a capture,
   or finds in the tree. "Heavy Strike staggers a parrying enemy in a two-player session" is
   a goal; "implement stagger" is not. A phase that ends at a console cheat is not done.
2. **State and what is ruled out**: the HEAD it starts from, what earlier passes changed or
   removed, the diagnosis so far and **"verify this before acting on it"**.
3. **What to read**: named files, a task range in a plan, a wiki page. Never "read the whole
   plan" (plans run thousands of lines; that is 150k tokens before the first edit).
4. **Scope**: the folder, branch, editor port and owned paths (see `ue-agent-lanes`); whether
   it may build, open the editor or commit; the files that are the owner's and must not be
   touched; **"do not spawn sub-agents or workflows; one heavy process at a time."**
5. **Report-back shape**: what landed (commit SHAs), what was verified and how, what is UNRUN,
   what it refuted, what it left out and why, the checklist of owed paperwork as DONE/NOT DONE.

Also in every brief: **never block on a call longer than about two minutes**. Long builds,
suites and cooks run in the background with output to a file, polled with a cheap
`tail`/`grep -c`. A subagent's prompt cache can expire during a long blocking call, and its
next turn then re-writes its whole context. Grep finished logs; never read one whole.

## Choose the model by the work

| Work | Model |
|---|---|
| Checking a plan's anchors against the tree, transcribing settled decisions, doc edits, reading files and reporting facts, gate and packaging runs | A cheap model |
| Implementation, review, writing a plan or spec, art-direction review | The strong model |

If you cannot say what judgement the agent must exercise, it is mechanical. Downgrade it.
One session that never passed a model ran about 25 subagents and 3.8M tokens on the strong
model; seven of them were doing grep-and-compare.

**Collapse stages.** Check anchors, fix the plan, implement is three agents reading the same
document. One agent that checks its own task's anchors, fixes what is wrong and builds it saves
two full reads. Split only when the stages need different models or a human decision sits
between them.

## Lookups and audits

- Brief a lookup with an explicit boundary: **"answer the question; change no files, run no
  build, make no commit."** A helper asked for "the build commands" once came back having done
  the whole task, with commits its parent had not made. Prefer a read-only agent type.
- **An audit is a photograph.** On a busy day four of five morning diagnoses were wrong by the
  afternoon: renamed classes, refunds that had become structural, a log line that fires before
  the thing it watches exists. Name the HEAD an audit was taken at, and brief every implementer
  to verify the diagnosis first. **A refuted finding is a result**: the thing is already right.
- A class name the tree no longer has is the strongest staleness tell; everything around it is
  suspect.

## Reading a report

The detail is in **references/reading-reports.md**. The short form:

- **Tests that did not run are UNRUN**, in your summary to the owner, until they have run.
  Compiled is not passed: a test written while the editor held the DLLs was reported as done,
  and failed on its first run because it could never have passed.
- **Check claims against the tree, not the commit message.** `git show --stat` and the diff
  say what landed; the message says what was meant.
- **A count dropping to zero is not a fix until you know why it dropped.** "Floating props: 40
  to 0" was the dressing tool no longer placing props at all.
- **An absence in a log is evidence only if the success path logs.**
- **"Fixed" comes from the owner or from the log**, never from a theory. When the owner reports
  a fault, instrument the running game, reproduce once and read one line before offering any
  explanation. Their observation is the finding.
- Do not relay an agent's code-reading as a finding, and do not propose work from it. A claim
  about runtime behaviour is a finding once it is seen at runtime.
- Do not re-verify what was reported accurately. Check when something is suspicious: a claim
  that contradicts itself, a skipped gate, a number that cannot be right.

## Agent lifetime

- **Do not resume a long-idle agent for a small follow-up**; its first message re-writes most
  of its context. Send a fresh, scoped agent. Resuming is right for genuinely interrupted work.
- **Retire long-context agents** at a clean pushed commit and restart fresh from their report.
- Stopping a parent may not stop its children. After stopping an agent, list running agents
  and stop any it left behind. A brief that did not forbid sub-agents once produced a fan-out
  of nine, each running `git status` on a huge repo, and pinned the owner's machine.
- Concurrent agents share one scratchpad directory. Give each a filename prefix, and never let
  two agents edit one document: the last writer wins and neither knows.

## When the owner corrects you

A correction is a lesson with a reason. Write it down **once, in the place that will enforce
it**, with a **Why:** line naming the failure, and a **How to apply:** line. Where each kind
belongs is in **references/corrections-become-rules.md**. If the owner repeats a requirement,
you missed it the first time: implement what they said; do not offer them options on a settled
question.

## Red flags

| You see | It means |
|---|---|
| A brief with no model, no owned paths or no "no sub-agents" line | The defaults are the strong model, everything, and fan-out |
| "Read the plan / the spec first" with no range | 150k tokens before the work starts |
| A summary that says "done" over tests that never ran | A guess reported as a result |
| A rule count of zero in a report, with no explanation | Possibly nothing being checked |
| A diagnosis quoted from this morning's audit | A photograph of a tree that has moved |
| An agent blocking on a five-minute suite | Its cache is expiring |
| The same correction from the owner twice | A rule with no home |
