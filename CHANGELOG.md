# Changelog

All notable changes to this skill. Versions follow [semver](https://semver.org/): a change
that alters what the skill tells an agent to do is at least a minor bump.

## 0.3.0

Four skills for working with AI agents on an Unreal project. Each rule is paired with the
generalised failure that taught it.

- `skills/ue-agent-lanes`: one writer per tree; a lane is a worktree, branch, editor port and
  owned paths; build worktrees only through the main engine path; stop only PIDs you started;
  protect the developer's uncommitted changes; merge at commit boundaries and re-bake generated
  levels; light gates for content, the full gate before main. References: headless editor
  work (`-ExecutePythonScript` quits the next tick unless kept alive, `-ExecCmds` never quits,
  every script quits itself, level-saving scripts assert the open level, the editor cap and
  the hung-on-exit watchdog) and merging, gating, the overnight heartbeat and retiring agents.
- `skills/ue-agent-briefs`: the five-part brief (goal, state and what is ruled out, what to
  read, scope, report-back shape) with a template; model choice; lookups with a read-only
  boundary; audits go stale; reading reports (UNRUN tests, the tree over the message, a count
  falling to zero); agent lifetime; corrections become rules with a Why, in the place that
  enforces them.
- `skills/ue-visual-review`: fixed review shots with the engine's AutomatedPerfTesting static
  camera and sequence tests in an always-loaded review sublevel; eyewitness passes; realism
  rules; art-direction rounds by a fresh agent; notes folded into issues per kit or tool;
  one harness run at the end of a push.
- `skills/ue-pcg-fixtures`: a review note becomes a fixture canvas; one level per system; an
  `AFunctionalTest` per canvas over the bake check; expected failures with an issue, never a
  loosened rule.
- `ue-engineering-paradigm` `references/content-pipeline-and-taxonomy.md`: "The contract is the
  interface" (frame, declared length, tags, slots, sockets and connectors, volumes, collision;
  tight multi-box occupancy; how a hand-made piece supplies the contract), plus two red flags
  and the index row.
- `evals/`: six tasks for the new skills (08-13). UNRUN: `RESULTS.md` says how to run them.

## 0.2.0

- `references/content-pipeline-and-taxonomy.md`: extend the engine's pipeline with one shared
  vocabulary instead of building beside it. Assets describe themselves through DCC custom
  properties, an Interchange pipeline checks them and copies them in, the asset registry
  stores only what nothing else records, faceted gameplay-tag taxonomies (form/style/trait)
  replace metadata in names, a Content Browser filter is built from the taxonomy, and content
  is organised as library plugins with dependent plugins that append to them.
- `SKILL.md`: two red flags (metadata in names or sidecar manifests; bespoke importers or
  browsers) and the reference index row.

## 0.1.0

First release.

- `SKILL.md`: the three layers (C++ machine, Blueprint grammar, data adjectives), the boundary
  test, a "where does this go?" decision procedure, one Blueprint child per verb, the six-rule
  node contract, the C++-instinct table, counters for designs that still fail the test, red flags.
- `references/`: node contract, abilities as graphs, data assets and validation, content
  velocity, composition and god classes, traps, reviewing, anti-patterns gallery.
- `templates/`: latent ability task, instantaneous node, BlueprintNativeEvent seam, required/ref
  pins, validated primary data asset, editor validator, Asset Manager discovery.
- Engine claims checked against UE 5.8 source; `scripts/check-citations.mjs` keeps them honest.
- `evals/`: seven golden tasks with rubrics, and baseline vs with-skill results.
- Packaged as a Claude Code plugin with its own marketplace manifest.
