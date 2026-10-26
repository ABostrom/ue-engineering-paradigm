# Building fixture levels

## Where they live

- In an **editor-only plugin** that never cooks. A fixture level references every kit; a
  runtime library plugin that may not reference its users would fail data validation's
  reference rules if it held them.
- One level per system. A row of canvases along one axis, far enough apart that no keep-out,
  span or lead from one reaches the next.

## Generating the canvases

Write the canvases as **data rows in a script**, not by hand in the editor, so a level can be
rebuilt after a kit changes:

- The case name, the district or kit, the fewest buildings that show the spot.
- A function that puts the case's own pieces **on sockets** (a torch bracket on its host's
  mount socket) or **on the grid** (the third bay of the second storey), the same way the
  real tools would. A piece floating at hand-typed coordinates tests a layout no tool makes.
- A marker for the close camera and for any joint expectation.
- The expectations, and a note in words.
- The script logs a `SETUP` line for every socket or bay a case needed and did not find, so a
  kit change that breaks a canvas shows as a setup failure, not a system failure.

Raise the canvas's buildings with the **same tools** the real levels use, then convert what
they laid into the canvas's hand-placed pieces (minus whatever the system under test lays
itself). Every piece then stands where a tool would put it.

Run the build script headless, one level per editor run, and have it quit (see
`ue-agent-lanes` references/headless-editor.md).

## The test

An editor-only `AFunctionalTest` subclass (`Engine/Source/Developer/FunctionalTesting/Classes/FunctionalTest.h`)
in an editor module, so it runs in the editor world without PIE:

1. Generate the canvas's PCG volume locally (`GenerateOnDemand` trigger, see
   `EPCGComponentGenerationTrigger` in `Engine/Plugins/PCG/Source/PCG/Public/PCGComponent.h`).
2. Wait until the spawned instances and spline meshes **hold still** for a number of frames.
   Generation can take several ticks; a check that runs on the first tick sees half a canvas.
   Give up after a bound and say so.
3. Log one perf line per canvas: generation time, instance count, component count.
4. Run the bake check in-process, scoped to the volume's bounds, writing a JSON report.
5. Hold the report to the expectations; log every failure and every example location with
   the canvas id.

**Test names.** A map can carry a `TestPathOverride` asset registry tag, which the functional
test framework uses as the test path prefix instead of the map's package path
(`MapPackageToAutomationPath` in `FunctionalTest.h`). Set it so the tests group as
`<Project>.Fixtures.<Level>.<id>` and a run can select one system by name.

## The PCG graph under test

The fixture's graph runs **the system alone**: read the canvas's pieces (Get Actor Data by tag,
as point data with their tags as attributes), read the footprints, build keep-outs from the
pieces, then the system's own chain. Nothing that needs the full level (the playable area,
boundary checks) runs.

## Traps

- **Get Actor Data must be limited to the volume** (in UE's PCG, "Must Overlap Self"), or every
  canvas's volume reads every canvas's pieces.
- **A tool that expects level context logs an error with none.** A fixture graph must not nest
  a tool that needs the playable area or another level-wide actor; a logged error fails a
  functional test.
- **Editor Python deprecation warnings are logged as errors** in some setups, which fails a
  functional test that runs Python. Silence them in the check's fixture mode.
- **Options that start with a minus.** A bounds argument like `-1200,...` reads as a new option;
  write `--bounds=<value>`.
- **Fixtures go stale with the kit.** After a kit re-import, re-run the build script for every
  fixture level, then the tests, then the shots. A canvas built on old pieces tests old pieces.
