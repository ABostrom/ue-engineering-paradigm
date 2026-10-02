# Traps that look like working code

Most of these share one shape: **a default stands in for a runtime value, and the wrong answer
looks exactly like a right one.** No error, no warning - a zero, a stale value, or nothing.

## Defaults and the CDO

### 1. Container defaults freeze in saved Blueprints

Properties are **delta-serialized** against the archetype: a value is saved only if it is not
`Identical` to the default (`SerializeVersionedTaggedProperties` in
`Engine/Source/Runtime/CoreUObject/Private/UObject/Class.cpp`). A `TArray`, `TSet`, `TMap` or tag
container is compared **as a whole**. So once a Blueprint child has changed a container at all -
added one blocked tag of its own - it saves the *entire* container, and an entry added later to
the C++ parent's default **never reaches it**.

Example: you add `State.Paused` to `UMeleeAbility::ActivationBlockedTags` so no ability starts
while paused. `BP_GA_Dodge`, which had added `State.Stunned` to its own list a month ago, ignores
the pause. Silently. Re-saving each asset "fixes" it and leaves the trap armed for the next tag.

**Rule:** a fact that must hold for **every** subclass goes in **code**, inside the function that
reads it (the activation gate checks the pause), not in a container an author can override.
Per-verb entries stay in the container.

**Catch it:** iterate every non-abstract subclass **including Blueprints** (`GetDerivedClasses`
after loading the content), ask each CDO the question its own activation asks, and assert the
answer flips when the fact is raised. A spot check on the native class passes while the shipped
content does not.

### 2. Script-set Blueprint defaults can vanish

Writing a value onto a Blueprint's generated-class CDO from an editor script and saving can read
back correctly in the same session and be gone after a reload. Edit Blueprint defaults through
the Blueprint and **compile it before saving** (for example
`unreal.BlueprintEditorLibrary.compile_blueprint` in editor Python), then reload and check. If a
Blueprint default "did not stick", the answer is a compile - never a hardcoded path in C++ as a
workaround.

### 3. A granted ability spec's `Ability` is the CDO

`FGameplayAbilitySpec::Ability` is documented as "Always the CDO"
(`Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayAbilitySpec.h`).
For instanced abilities, any field written during activation lives on the **instance**. Reading
it from the spec reads the constructor value - and a test asserting on it can pass against
zeroes. Read runtime state from `FGameplayAbilitySpec::GetAbilityInstances()`. The spec's
`Ability` is only good for asking the class about itself (tags, costs, classification).

### 4. A Blueprint CDO that fails `IsDataValid` is born in error

Compiling a Blueprint runs `IsDataValid` on its CDO
(`Engine/Source/Editor/Kismet/Private/BlueprintCompilationManager.cpp`). A strict validator that
expects instance data refuses the empty CDO, the Blueprint compiles to an error state, and every
load and cook then errors - found at the cook, far from the cause.

Which fix depends on **where the content lives**:

- **Content lives on instances** (data asset instances, level-placed actors): return
  `NotValidated` for `RF_ClassDefaultObject` and keep the real rules strict for instances.
- **Content lives on the class default** (an ability Blueprint child whose tunables are its
  defaults, a data-only Blueprint): the CDO *is* the content, so validate it - but skip abstract
  classes and intermediate bases (`GetClass()->HasAnyClassFlags(CLASS_Abstract)`), which hold no
  content yet.

If an asset was saved in the error state, delete and recreate it after fixing the validator.

## Replication

### 5. A value equal to the archetype never travels

The server does not send a property, it sends a **difference**, and the baseline is the
object's **archetype**, not what the client holds: the replication changelist's shadow state is
initialised from `InObject->GetArchetype()` (`FRepLayout::CreateReplicationChangelistMgr` in
`Engine/Source/Runtime/Engine/Private/RepLayout.cpp`). This describes the generic replication
path; if your project runs Iris, verify its baseline behaviour separately.

So a **level-placed** actor reconfigured at runtime from data will not replicate a value that
happens to equal its CDO, even when the client's map-loaded copy differs. The comparison runs,
correctly concludes "unchanged", and filters the property. Values that differ from the CDO
travel, which hides the bug in most configurations.

Example: `BP_ArenaBoundary`'s default `Radius` is 2000; the map instance says 1200. The server
sets 2000 for a "Standard" match. Clients keep 1200. Pit (800) and Skirmish (1500) work.

**Fixes:**
- Force the full initial state for actors that are reconfigured at runtime:
  ```cpp
  void AArenaBoundary::OnSerializeNewActor(FOutBunch& OutBunch)
  {
      OutBunch.bOutWantsFullInitState = true;   // "even if they do not differ from Archetype baseline"
      Super::OnSerializeNewActor(OutBunch);
  }
  ```
  (`bOutWantsFullInitState` in `Engine/Source/Runtime/Engine/Public/Net/DataBunch.h`;
  `OnSerializeNewActor` in `Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h`.)
- Or spawn the actor at runtime from data instead of reconfiguring a placed one.
- Or make the placed instance match the CDO.

Ask of any placed actor that adopts a preset: *could this runtime value ever equal the CDO
while the client's map copy differs?*

**Test blindness:** a replication test whose fake wire copies every property is testing a wire
that does not exist. Filter on the archetype the way the engine does, or test with a real client.

### 6. A runtime-added component that does not replicate cannot be named

`UActorComponent::IsSupportedForNetworking()` is `GetIsReplicated() || IsNameStableForNetworking()`
(`Engine/Source/Runtime/Engine/Private/Components/ActorComponent.cpp`). A component **added at
runtime** is not name-stable, so unless it replicates it cannot be referenced over the network
at all - and anything that names it (an ability spec's `SourceObject`, an RPC parameter) arrives
as null on the client.

The wrong question is "does this component hold state worth replicating?". The right one is
"will anything ever **refer** to it across the wire?". If yes:
`SetIsReplicatedByDefault(true)`, even with zero replicated properties.

Symptom: works on the host, fails on joiners, and the server log mentions the object as not
supported for networking. Test with a real client; the listen-server host is not a network path.

## Renames and graphs

### 7. Renames orphan graphs

The C++ compiler protects a C++ refactor; nothing protects the Blueprints. Renaming a
`UFUNCTION` (or a latent task's factory) leaves every calling node orphaned - assets load and look
fine in the content browser, and fail only when compiled or run. Removing a parameter that a
graph typed a literal into leaves a compiler **warning**, not an error, so a "0 errors" gate
passes with it.

**Rule:** land the C++ change and the graph edits in **one change**, compile every affected
Blueprint (for example with the `CompileAllBlueprints` commandlet), or add
`+FunctionRedirects` / `+PropertyRedirects` / `+ClassRedirects` under `[CoreRedirects]`
(examples in `Engine/Config/BaseEngine.ini`). Never leave a window where the C++ has moved and the
graphs have not.

### 8. Orphaned nodes break graph tooling

Nodes whose function was renamed away can crash editor tools that walk a graph's nodes
(anything that asks an orphaned node for its category or function metadata). If you script graph
edits: delete orphaned nodes first, then author; read the graph back and **compile that
Blueprint** after each one, not once at the end. A text dump of a graph is a record, not a
re-playable script - it can be lossy. Compiling is what proves the wiring.

### 9. Dirty assets can hold the only copy of a fix

A rename with redirects resaves every asset that referenced the renamed thing; the fix lives in
the working copy. "Unexplained" modified Blueprints after a rename are often those fixes. Before
restoring or excluding a dirty asset, load or compile the **committed** version: if it is broken,
the dirty copy is the fix - commit it. Never restore on "nothing explains it".

## Source control

### 10. Restoring a Git LFS asset with `git show`

With `.uasset`/`.umap` in Git LFS, the git blob is a ~130-byte **pointer**.
`git show <rev>:<path> > file` writes that pointer over the asset - and `git diff` then reports
the file identical, because both sides are the same pointer. The editor then fails in ways that
look like something else.

Restore with `git checkout <rev> -- <path>` (runs the smudge filter) or `git lfs checkout <path>`.
Check an asset is real by size, or by its first line (a pointer starts
`version https://git-lfs`).

## Content paths in C++

### 11. "Just load it by path for now"

`LoadObject`, `ConstructorHelpers::FObjectFinder` or a `TEXT("/Game/...")` path in C++ decides
content in code, breaks on rename/move, and usually appears as a workaround for trap 2 (a
default that did not stick). Declare an `EditDefaultsOnly` property and assign it on the
Blueprint child; let a missing assignment fail loudly.
