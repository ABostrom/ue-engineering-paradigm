# Evals

Golden tasks that measure whether the skill changes what an agent **does**, not whether it reads
well. Each task in `tasks/` is a realistic prompt plus a rubric of what a good answer must and
must not do.

| Task | Exercises |
|---|---|
| `01-design-parry-verb` | The three layers, abilities as graphs, latent vs instantaneous nodes |
| `02-enum-switch-review` | One Blueprint child per verb, the config-schema trap, rename hazards |
| `03-enemy-type-needs-code` | Content velocity, primary assets, Asset Manager discovery, tag vs enum |
| `04-god-class-gamemode` | Composition, phases, one owner per fact, replicated composed pieces |
| `05-struct-param-api` | The node contract, `ExpandEnumAsExecs`, naming as API |
| `06-fallback-when-null-pr` | Validate at the edge, no fallbacks, no content paths |
| `07-replicated-value-never-arrives` | The archetype replication baseline |

## Method

1. **Baseline.** Give a fresh agent the task's *Prompt* section only, and tell it not to load
   skills or read files.
2. **With skill.** Give a fresh agent the same prompt, and tell it to read
   `skills/ue-engineering-paradigm/SKILL.md` and whichever references it finds relevant (or
   install the plugin and let it trigger).
3. **Score** both against the rubric: one point per *must* (half for partial); count every
   *must not* violation.
4. Record the result in `RESULTS.md`.

Never show the agent the rubric. A cheaper model (for example Sonnet) is a good test subject: if
the skill lifts a smaller model, it is teaching rather than decorating.

## Adding a task

Add one when an agent (with the skill) gets something wrong in real work. Write the prompt as the
situation arrived, in the running example's world (Parry, Dodge, Heavy Strike, enemies, weapons,
an arena), and write the rubric from what the right answer had to contain. Run the baseline first:
if the baseline already passes, the task does not discriminate - keep it only as a regression test.
