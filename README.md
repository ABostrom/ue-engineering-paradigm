# ue-engineering-paradigm

An opinionated architecture skill for Unreal Engine, for Claude Code and other agents that read
[Agent Skills](https://agentskills.io/specification). It answers **"where does each thing go, and
why?"** across C++, Blueprint and data, so that the next piece of content (a verb, an enemy, a
mode, an item) is made in a Blueprint or a data asset, with no programmer.

**Who it's for:** teams building gameplay in Unreal 5 with C++ and Blueprint (especially with the
Gameplay Ability System) who want agents, and new teammates, to make the same structural calls an
experienced lead would: designing a verb, a `BlueprintCallable` API, a data asset schema,
decomposing a god-class GameMode, or reviewing a change at the C++/Blueprint boundary.

## The paradigm in five lines

1. **C++ is a machine with no opinions; the Blueprint child is the grammar; data assets are the adjectives.**
2. A number or a pick from a set is **data**; order and branching are the **graph**; arithmetic is **C++**.
3. Every verb is **its own Blueprint child** drawing a graph from whole-verb C++ nodes - never a type enum plus a switch.
4. Everything Blueprint can see is **public API**: one job, no derivable inputs, exec outputs only where the author draws different things, the wire is the state.
5. **Validate at the edge, then trust** - no fallbacks, no content paths in C++, one owner per fact.

## Install

### As a Claude Code plugin (recommended)

The repository is its own plugin marketplace. In Claude Code:

```
/plugin marketplace add ABostrom/ue-engineering-paradigm
/plugin install ue-engineering-paradigm@ue-engineering-paradigm
```

Then restart Claude Code if prompted. Update later with `/plugin marketplace update ue-engineering-paradigm`.

The repository is private: you need read access on GitHub, and git on your machine must be able
to clone it (for example after `gh auth login`, or with an SSH key). If the short form fails, use
the full URL: `/plugin marketplace add https://github.com/ABostrom/ue-engineering-paradigm.git`.

### Manual install

Copy the skill folder into your personal or project skills directory:

```bash
git clone https://github.com/ABostrom/ue-engineering-paradigm.git
# personal (all projects)
cp -r ue-engineering-paradigm/skills/ue-engineering-paradigm ~/.claude/skills/
# or project-only
cp -r ue-engineering-paradigm/skills/ue-engineering-paradigm <your-project>/.claude/skills/
```

The skill triggers on its own when a task matches its description; you can also ask for it by
name ("use ue-engineering-paradigm").

## Contents

```
.claude-plugin/
  plugin.json                  plugin manifest
  marketplace.json             single-plugin marketplace (this repo)
skills/ue-engineering-paradigm/
  SKILL.md                     the paradigm, the decision procedure, red flags, index (loaded on trigger)
  references/                  loaded on demand
    node-contract.md           the six rules; latent tasks vs instant functions; naming; compile-time safety
    abilities-as-graphs.md     GAS abilities as Blueprint chains of C++ nodes; smells and conversions
    data-assets-and-validation.md  push decisions into data; Asset Manager; IsDataValid; instanced structs; tag vs enum
    content-velocity.md        audit: can each content kind be added with no code? blockers and fixes
    composition-and-god-classes.md thin GameMode/Controller; phases; one owner per fact; no invented systems
    traps.md                   CDO/default traps, replication baselines, renames, LFS restores
    reviewing.md               review checklist for C++/Blueprint changes and graphs
    anti-patterns-gallery.md   fifteen before/after pairs
  templates/                   short UE C++ with WHY comments
    LatentAbilityTask.h/.cpp   a WaitFor... UAbilityTask with one exec output per outcome
    InstantNode.h              plain BlueprintCallable nodes, ExpandEnumAsExecs
    NativeEventSeam.h          BlueprintNativeEvent extension seam
    RequiredPinNode.h          UPARAM(Required) / UPARAM(ref): unsafe graphs fail to compile
    ValidatedDefinition.h/.cpp UPrimaryDataAsset with fragments and IsDataValid
    EditorValidator.h/.cpp     UEditorValidatorBase registration validator
    DefinitionDiscovery.cpp    Asset Manager discovery and async load
evals/                         golden tasks, rubrics, baseline vs with-skill results
scripts/check-citations.mjs    verifies every cited engine path exists
docs/skill-authoring-guide.md  how to contribute
```

All examples share one running example: a melee action game with **Parry**, **Dodge** and
**Heavy Strike**.

## How it complements general Unreal skills

General Unreal knowledge skills (and the model's own training) cover **what is correct** in
Unreal: the right specifiers, the API signatures, how replication or the Asset Manager works.
This skill sits on top and covers **where things go**: which layer owns a decision, what a
Blueprint-facing API should look like, how a class should be decomposed, and which correct-looking
designs quietly make every future piece of content need a programmer. It deliberately does not
re-teach the engine; where it states engine behaviour, it cites the engine source.

In our evals (see `evals/RESULTS.md`) a capable model without the skill scored 64% against the
rubrics and made three "must not" mistakes; with the skill it scored 97% with none. The
difference is concentrated in exactly the opinionated calls: where the sequence of an ability
lives, how a node forks, and what a variant is.

## Engine version and citations

Written against **Unreal Engine 5.8**. Engine claims cite paths relative to the engine root
(`Engine/Source/...`, `Engine/Plugins/...`, `Engine/Config/...`). Check them against your engine:

```bash
UE_ENGINE_ROOT=/path/to/UE_5.x node scripts/check-citations.mjs
```

The script proves the cited files exist; when one moves after an upgrade, re-read the claim.

## Contributing

Read `docs/skill-authoring-guide.md`. In short: keep `SKILL.md` lean and put depth in
`references/`; use the running example; generalise the lesson and drop the project story; verify
every engine claim in source and cite it; and test a change on agents - run an eval task without
the skill, change the skill, run it with the skill, and record the result in `evals/RESULTS.md`.
Bump the version in both manifests and add a `CHANGELOG.md` entry.

## License

MIT - see `LICENSE`.
