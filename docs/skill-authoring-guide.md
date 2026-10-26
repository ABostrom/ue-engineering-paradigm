# Contributing to the skill

House rules for changing any skill under `skills/`. They exist so each skill stays short, true
and testable.

## 1. Shape

- `SKILL.md` is loaded whenever the skill triggers. Keep it **at or under ~200 lines**: the
  paradigm, the decision procedure, the red flags, and an index. Depth goes in `references/`.
- One topic per reference file. A reference is loaded on demand, so it can be long, but it must
  be reachable in **one hop** from `SKILL.md` (listed in the reference index).
- `templates/` hold short, compilable-looking UE C++ with comments that say **why**, not what.

## 2. The description

The frontmatter `description` decides whether an agent loads the skill at all. It lists
**situations and symptoms only** ("Use when designing ... or reviewing code with a type enum plus
switch ..."). Never summarise the skill's advice in it: agents follow a summary instead of
reading the body. Max 1024 characters.

## 3. One running example

Every example uses the same neutral game: a third-person melee action game with **Parry**,
**Dodge** and **Heavy Strike**, plus enemies, weapons and an arena mode where a second domain is
needed. The agent-workflow skills use the same game's production: fortress arenas dressed by
PCG, a DCC kit pipeline, and lanes for gameplay, arena, look and art. Do not
introduce a new genre for one example; consistency is what lets a reader skim.

## 4. Generalise the lesson, drop the story

A contribution usually starts from a real failure in a real project. Keep the **general Unreal
lesson** and the mechanism; drop project names, class names, file paths, dates, people and tool
names. "A Blueprint child that had added its own blocked tag ignored a tag later added to the C++
parent" is a lesson. "On the 3rd our dash ability walked through the pause" is a story.

## 5. Every engine claim is verified

- Before writing a claim about engine behaviour or an API, open the engine source and read it.
  Do not write from memory or from another agent's summary.
- Cite it as a path relative to the engine root: `Engine/Source/...`, `Engine/Plugins/...` or
  `Engine/Config/...`. Never an absolute path from your machine.
- Quote the engine's own comment where one exists; it survives refactors better than line numbers
  (so do not cite line numbers).
- If you cannot verify a claim, soften it ("verify on your engine version") or drop it.
- Run the checker:
  ```
  UE_ENGINE_ROOT=<dir containing Engine/> node scripts/check-citations.mjs
  ```
  It proves cited files exist. It does not prove the claim; when a path moves on an engine
  upgrade, re-read the claim against the new source.

## 6. Test changes on agents (RED, GREEN, REFACTOR)

A change to what the skill says is a change in behaviour, so it is tested like code:

1. **RED.** Pick or write a task in `evals/tasks/`. Run it on a fresh agent **without** the skill
   and record what it gets wrong against the rubric.
2. **GREEN.** Change the skill to address those specific failures. Run the same task **with** the
   skill. It should now pass the rubric.
3. **REFACTOR.** Look for new rationalisations in the with-skill answer and close them. Re-run.
4. Record the result in `evals/RESULTS.md`.

Do not add content for a hypothetical failure no baseline showed.

## 7. Release

Bump `version` in `.claude-plugin/plugin.json` and `.claude-plugin/marketplace.json`, and add a
`CHANGELOG.md` entry. Installed copies update when users run `/plugin marketplace update`.
