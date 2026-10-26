# The review harness: AutomatedPerfTesting

The engine's **AutomatedPerfTesting** plugin (`Engine/Plugins/Performance/AutomatedPerfTesting`)
already does what a review needs: named shots, the same every run, from a real build, with a
cost per shot. Use it instead of a capture script.

## What it provides

| Piece | Where | What it does |
|---|---|---|
| `AAutomatedPerfTestStaticCamera` | `Engine/Plugins/Performance/AutomatedPerfTesting/Source/AutomatedPerfTesting/Public/StaticCameraTests/AutomatedPerfTestStaticCamera.h` | A placed `ACameraActor` with a `CollectionName` |
| Placed static camera test | `Engine/Plugins/Performance/AutomatedPerfTesting/Source/AutomatedPerfTesting/Private/StaticCameraTests/AutomatedPlacedStaticCameraPerfTest.cpp` | Gathers every static camera in the world (`GetAllActorsOfClass`), then visits each: warm up, soak (CSV profile), cool down, screenshot |
| Static camera settings | `UAutomatedStaticCameraPerfTestProjectSettings` in `Engine/Plugins/Performance/AutomatedPerfTesting/Source/AutomatedPerfTesting/Public/StaticCameraTests/AutomatedStaticCameraPerfTestBase.h` | `MapsToTest`, game mode override, screenshots on or off, warm-up, soak and cool-down times |
| Sequence test | `UAutomatedSequencePerfTest` in `Engine/Plugins/Performance/AutomatedPerfTesting/Source/AutomatedPerfTesting/Public/AutomatedSequencePerfTest.h` | Plays a Level Sequence; `MapsAndSequencesToTest` pairs a map with a sequence |
| Gauntlet nodes | `Engine/Plugins/Performance/AutomatedPerfTesting/Build/Scripts/AutomatedStaticCameraPerfTest.cs` (and siblings) | `AutomatedPerfTest.StaticCameraTest` and friends, run through `RunUAT RunUnreal` |

If your engine
ships ushell, `.perf test camera` and `.perf test sequence` build the same Gauntlet command.

## Setting it up

1. **Enable the plugin** for the game target, so the packaged Development build carries the
   camera class and the test controllers.
2. **Settings in a plugin's own config**, not the project's main ini, if the project owner
   edits that file by hand. Editing them in Project Settings writes to the project's default
   ini; move them back.
3. **A review game mode.** The plugin ships a bare game mode (`AAutomatedPerfTestGameModeBase`, `Engine/Plugins/Performance/AutomatedPerfTesting/Source/AutomatedPerfTesting/Public/AutomatedPerfTestGameModeBase.h`):
   no match, no players, no HUD. Anything the shots need (a lighting preset, weather) must come
   from something that runs without your game mode, such as a world subsystem reading a
   console variable.
4. **A review sublevel per level**, streamed always-loaded, holding only review actors: the
   static cameras and an idle `ALevelSequenceActor` for the flythrough (which is what makes the
   sequence cook). A bake or dressing pass rewrites the persistent level and never touches the
   sublevel.
5. **Generate the cameras from a script**, from frames you can name and re-derive (the gameplay
   camera's frame, an overview, eye height in the streets looking in, a facade face-on), and
   keep hand-placed cameras the script did not make. Tag generated ones so a re-run replaces
   only those.
6. **Names**: `<Level>_<NN>_<Shot>` (`Arena_01_Gameplay`, `Arena_02_Overview`,
   `Arena_03_Gatehouse`). Set the actor's **object name**: actor labels are editor-only data
   (`ActorLabel` is under `WITH_EDITORONLY_DATA` in
   `Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h`), and the test names its
   screenshot and CSV after `GetActorNameOrLabel()`.

## Running it

1. Build, cook and stage the game (the test runs against the staged build). Any camera, level or
   sequence change needs a new cook.
2. One `RunUnreal` per level, per lighting preset, per kind (camera or sequence), passing the
   preset as the plugin's console-variable option so it is set before the map loads.
3. Copy the screenshots to a committed, dated folder (`Renders/review/<date>/<Level>/<Preset>/`)
   and the CSVs and client logs somewhere local. Gauntlet's temp output is overwritten by the
   next run.
4. Summarise per camera: median frame and GPU time over the soak (drop the first frames); per
   flythrough cut, the 95th percentile too, because a hitch shows there and not in the median.
   Compare against the last baseline; treat a sub-half-millisecond change as noise until it
   repeats, and note anything else that was using the GPU.

## Traps

- **Cameras must be loaded at setup.** The test gathers cameras once; a sublevel streamed in
  later contributes nothing. Always-loaded.
- **A merge or re-bake that takes another branch's level can drop the sublevel's streaming
  entry.** Make the camera script idempotent and re-run it after such merges.
- **Gauntlet runs the client deterministic** (a fixed step), so "seconds" of warm-up and soak
  are frames. Frame and GPU times are still real.
- **Quote every argument in PowerShell.** An unquoted dotted option is split at the dot and the
  run fails with a misleading error.
- **The shots are what the camera sees with no players**: no characters, no gameplay camera
  effects (a wall fade, for example). A tall wall that the gameplay camera would fade will block
  the gameplay shot.
- **Read the client log for proof** the preset applied (have the subsystem log one line when it
  applies a look); a screenshot under the wrong lighting looks plausible.
