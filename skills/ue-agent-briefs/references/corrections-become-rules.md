# Corrections become rules

When the developer corrects an agent, the correction is the most expensive information in the
session: it cost a failure to learn. If it lives only in the conversation, the next session
makes the same mistake.

## The shape of a rule

```
<The rule, one line, imperative.>

Why: <the failure that taught it, generalised: what happened and what it cost>.
How to apply: <when it bites, and the concrete move>.
```

The **Why** is not decoration. A rule without one gets applied where it does not fit and
dropped where it does, because the reader cannot tell what it protects. "Stop only PIDs you
started" is followed; "Stop only PIDs you started. Why: the build tool's process name matched
the IDE installing its engine plugin, and killing it cost half an hour" is understood, and
extends itself to MSBuild and the editor.

## Where each lesson goes

| The lesson is | It goes in | Example |
|---|---|---|
| A fact about this repo every session needs | The project rules file (`CLAUDE.md`) | where the source of truth lives; which folders never to search |
| How to work with this developer | The agent's memory | "take the editor without asking, unless they say they are testing" |
| A procedure with checks, used in one kind of task | A skill | how to brief a lane; how to review a commit |
| Engineering truth about the code: who owns what, which store, a confirmed trap | The project's wiki or docs, on the subsystem's page | "a non-World-Partition level's PCG generates only from an editor script" |
| Something that must never be skipped | A hook | a pre-commit check that stale docs were touched; a pre-push refusal while paperwork is owed |
| A general Unreal lesson, true beyond this project | A shared skill, generalised | this plugin |

**A hook enforces presence, not truth.** A hook can prove a page was touched; it cannot prove
the page is right. Pair it with the skill that says what "right" means.

**Prefer the place that enforces.** A rule that must hold every time and can be checked
mechanically belongs in a hook or a test, not in memory. A rule that needs judgement belongs in
a skill a reviewer loads. Memory is for how to work with a person.

## Keep the set small and true

- **One home per lesson.** Link to it from elsewhere; do not copy it. Two copies drift.
- **Update, do not append.** When a later correction sharpens a rule, rewrite the rule and add
  the new reason to its Why. A rule with three dated amendments is three rules arguing.
- **Delete rules that no longer hold.** A stale rule is worse than none, because it is trusted.
- **Generalise before sharing.** For a shared skill, keep the mechanism and drop the story:
  no project names, issue numbers, dates, people or paths.

## Repeated requirements

If the developer says the same thing twice, the first answer missed it. Check the conversation and
the docs before asking; implement what they said; flag a genuine conflict in one sentence. When
they name a specific target (an actor, an asset, two things that should be one), the deliverable
is that target changed, not an investigation of why it cannot be. Ask them to **verify finished
outcomes**, never to **re-specify decisions**.
