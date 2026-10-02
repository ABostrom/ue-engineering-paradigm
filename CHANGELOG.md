# Changelog

All notable changes to this skill. Versions follow [semver](https://semver.org/): a change
that alters what the skill tells an agent to do is at least a minor bump.

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
