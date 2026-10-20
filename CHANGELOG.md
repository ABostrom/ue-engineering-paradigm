# Changelog

All notable changes to this skill. Versions follow [semver](https://semver.org/): a change
that alters what the skill tells an agent to do is at least a minor bump.

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
