# Reading an agent's report

An agent's mistakes come back in the same confident voice as its findings. These are the
checks that caught real ones. Use them when something looks off; do not re-run every claim as
routine (re-checking accurate reports wastes the time the delegation saved).

## Tests

- **UNRUN is a status.** A test that compiled but never ran is a guess. When the editor holds
  the module DLLs, nothing links and nothing runs; four tasks reported "done, the link and
  suite outstanding" were summarised as done, and the first suite run found a test that could
  never have passed (it asserted on a component a bare test world never creates). Say "N tests
  written, UNRUN" until they run, and expect the first run after such a window to find
  something.
- **Check the build's exit code before believing a test result.** A mutation that failed to
  compile left the previous binary on disk, and the chained test run printed a plausible result
  from the previous mutation.
- **A suite run measures the binaries on disk.** Started while another agent was relinking, or
  against binaries that predate the change, its number belongs to no commit.
- **A test count is not progress.** It is not evidence the feature works for a player either:
  a green suite says nothing about feel, readability or a Blueprint graph no test loads.
- **Could the test fail?** An assertion against its own input, a field set by hand and read
  back, a test whose assertions all pass in one direction: each passes without the code doing
  its job.

## Claims about the tree

- **The tree, not the message.** Read `git show --stat <sha>` and the diff. A message can
  describe a change the commit does not contain, or miss one it does.
- **Check `git log` and file timestamps before redoing work.** Commits the agent did not make,
  or binaries newer than its last edit, mean someone else already did it.
- **Say which measurement.** A working-tree measurement and a stored-blob measurement can
  honestly disagree (line endings under `core.autocrlf` are the classic case). Two agents
  measuring different ones are not contradicting each other.

## Counts and logs

- **A count dropping to zero is not a fix until the reason is known.** In the running example,
  the arena bake check's "floating props" rule went from 40 to 0 after a dressing change. The
  change had stopped the tool placing props at all. Ask what the count is a count *of*, and
  check the denominator (props placed) moved the way it should.
- **A count moving the right way can be a checker that stopped looking**: a rule scoped to the
  wrong bounds, a filter that matches nothing after a rename.
- **Absence in a log is evidence only if the success path logs.** "Watching 0 enemies" was
  logged at world start, before any enemy spawned; the same log bound all of them later.
- **A warning in a log is a finding until shown otherwise.**
- **Scope a negative to the path it was measured on.** "The material never rendered" was true
  of one render path and false of the asset.

## Claims about runtime behaviour

- A claim about what happens in play, made from reading code, is a guess. Label it as one in a
  single line, or leave it out. Do not schedule work from it.
- When the owner reports a fault, their report is the finding. Add one log line that states
  what the failing system did, build once, reproduce once, read it. Theories before that cost
  hours; the instrumented run usually answers it in one go.
- When one symptom has had two causes, assume a third layer, not vindication.
- Never replace a working path wholesale before the new one has worked once. Changes stacked on
  an unverified base mask each other.

## Visual claims

- **Look at the frame yourself.** A feature reported built was indistinguishable from the
  surface under it in the capture.
- A green bake check proves the rules it has. It does not prove the place looks real (see
  `ue-visual-review`).
