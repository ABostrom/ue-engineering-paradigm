# Reviewing a C++/Blueprint change

Review in this order. The early questions are the expensive ones to get wrong; naming and
style come last because they are cheap to fix - except on the Blueprint surface, where a name
is public API.

## 1. Layering

- [ ] Does any C++ symbol carry a verb's name (`ResolveHeavyStrikeVsParry`, `DodgeThenRecover`)?
      A sentence living in C++ that a graph should hold.
- [ ] Does a C++ `ActivateAbility` run the sequence (callbacks, timers, `bWaitingFor...` flags)?
      Or does the Blueprint child's graph draw it?
- [ ] Is there a type enum plus a switch choosing behaviour (in C++ or as a Blueprint `Switch`)?
- [ ] Is there a data list of *steps* interpreted by C++?
- [ ] Is there arithmetic in a graph (a multiply after a Break, a clamp, a lerp on a result)?
- [ ] Is there a C++ class whose only content is a constructor?
- [ ] **The falsifiable test:** could the *next* verb/enemy/mode in this change's family be
      made in Blueprint or data alone? If not, what node or field is missing?

## 2. Data and content

- [ ] Any `LoadObject`, `FObjectFinder`, `StaticClass()` content choice, or `/Game/` path in C++?
- [ ] Any hand-maintained list of content (types, classes, assets) that the Asset Manager or
      asset registry could answer?
- [ ] Any fallback, stand-in, default asset or "repair" for missing authored data? Should be
      `IsDataValid` / a validator plus a clear runtime refusal.
- [ ] Scattered `if (!X) return;` past the validation edge? Should be `check`/`ensure`, or fixed
      at the source.
- [ ] Tunables in constructors instead of `EditDefaultsOnly` properties on the Blueprint child?
- [ ] New gameplay tags: is the set open and design-driven at both ends (tag), or closed and
      switched on in C++ (enum)? Does any C++ read the new tags?
- [ ] One shared asset (DataTable, ability list) that everyone will edit?
- [ ] New strict `IsDataValid`: does it skip the CDO?

## 3. The Blueprint surface (node contract)

- [ ] Latent only where time passes; tasks named `WaitFor<What>`; cleanup in `OnDestroy`; no
      broadcast from the factory.
- [ ] Names `<Verb><Object>[<Qualifier>]`, no narration, acronyms uppercase. Pin names and
      `Category` strings too.
- [ ] No input the node can derive; no enum converted into an actor; no struct pins a designer
      must Make/Break.
- [ ] Exec outputs exactly where the author draws different things; no half-null pins.
- [ ] No state passed between nodes through members.
- [ ] Anything that must be wired is `UPARAM(Required)` or `UPARAM(ref)`.
- [ ] Renamed or removed `UFUNCTION`s, parameters or enum values: are the calling graphs
      re-authored and compiled in the same change, or covered by CoreRedirects?
- [ ] New `UFUNCTION`s with no Blueprint caller and no planned one? Drop the specifier; every
      exposed function is API you maintain.

## 4. Ownership and composition

- [ ] Does a new job land in a GameMode/Controller/Character body instead of a component,
      subsystem or phase?
- [ ] Does a new list/registry/manager duplicate a fact something already holds?
- [ ] Does a comment say two things "must agree"?
- [ ] Does any client-side code read the GameMode?
- [ ] Is a world-type list standing in for asking whether a dependency exists?

## 5. Replication and defaults

- [ ] Components added at runtime that anything references over the network: replicated?
- [ ] Placed actors reconfigured at runtime: can a value equal the CDO while the client differs?
- [ ] New entries in a base class's container default that every subclass must honour: should
      the rule be in code instead?
- [ ] Tests reading ability fields through the spec's `Ability` (the CDO)?
- [ ] Was it tested with a real second client, not only as the listen-server host?

## 6. Evidence

- [ ] Every Blueprint touched by the change compiled with no errors **and** no new warnings
      (removed-pin warnings are real).
- [ ] The change was exercised in play, end to end, not only by tests. Tests check outcomes
      ("the parried attacker staggers"), not mechanisms ("the flag was set").
- [ ] Binary assets in the diff: are the dirty ones intended, and are any missing that a rename
      resaved?

## What not to ask for

Findings must name a **real path** in play or set-up that breaks. Do not request robustness
machinery for hypotheticals ("what if a listener removes itself mid-broadcast?") - at most a
`checkf` stating the rule. Do not request extension points for a plan that has not arrived.

## Reviewing graphs

Blueprints are reviewable. The editor's **Blueprint Diff** compares a graph against a
source-control revision node by node, and there is a merge tool for Blueprint conflicts. Use it;
do not accept "graphs can't be reviewed" as a reason to keep sequence in C++.

What is weaker is review *outside* the editor: a pull-request diff shows a binary blob. Help it:

- Keep graphs short (whole-verb nodes do this) and one verb per asset.
- Write the graph's shape in the PR description (`WaitForIncomingHit -> Parried: ApplyStagger ->
  OpenRiposte`).
- A text projection of the wiring (from your editor tooling) is useful for agents and for
  spotting drift. It is a convenience, not a missing capability - and it can be lossy; compiling
  is what proves the wiring.
- Review that the graph holds sequence and nothing else: no math nodes, no Break-then-Branch on a
  node's result, no `Switch on E<Type>`.
