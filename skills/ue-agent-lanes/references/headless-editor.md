# Headless editor work

Agents drive the editor from the command line: bake a level, rebuild PCG graphs, import a kit,
take captures. Each run must start the right way, do its work on the right level, write a
result an agent can read, and **exit**. An editor that never exits holds DLLs, a GPU context
and sometimes a level lock, and blocks every lane.

## Launching editor Python

There are two ways in, and they differ in when the editor quits.

| Launch | Quits | Use for |
|---|---|---|
| `-ExecutePythonScript="script.py args"` | The tick after the script returns, unless the script set keep-alive | Synchronous work that finishes inside the script call |
| `-ExecCmds="py script.py args"` | Never by itself | Work that completes over several ticks (PCG generation, shader compiles, async loads, screenshots) |

`-ExecutePythonScript` runs through the editor's Python executer, which requests exit on its
next tick once the script has run, unless keep-alive is set
(`Engine/Plugins/Experimental/PythonScriptPlugin/Source/PythonScriptPlugin/Private/EditorUtilities/EditorPythonExecuter.cpp`).
The keep-alive flag's own comment: "If this is false (default), it will close the editor
during the next tick ... If this is true, it will not close the editor by itself, and you will
have to close it manually, either by setting this value to false again, or by calling a
function like unreal.SystemLibrary.quit_editor()"
(`Engine/Plugins/Experimental/PythonScriptPlugin/Source/PythonScriptPlugin/Private/EditorUtilities/EditorPythonScriptingLibrary.h`).

`-ExecCmds` queues console commands from the command line
(`ParseExecCmdsFromCommandLine` in `Engine/Source/Runtime/Engine/Private/UnrealEngine.cpp`);
nothing in it asks the editor to quit.

**The failure this prevents:** a bake launched with `-ExecutePythonScript` starts PCG
generation and returns; the editor quits on the next tick with nothing generated, and the
script's log says only that it started.

**Rules:**
- Latent work runs under `-ExecCmds="py ..."`, or under `-ExecutePythonScript` with
  `set_keep_python_script_alive(True)`.
- **Every script quits.** It ends by calling `unreal.SystemLibrary.quit_editor()` once its own
  work is done (on success and on failure), or the command list ends in `Quit`. A script that
  does not quit leaves an editor running until something kills it.
- The script writes a **result file** (`DONE`, counts, the level it saved) and the agent reads
  that, not the exit code. An editor that crashed during teardown, after its work was saved,
  still exits non-zero.

## Saving the right level

A level-saving script must **assert the open level** before it saves. Open the target, check
the editor world's package name equals it, and refuse (save nothing, write the reason) if it
does not, or if another level is still open.

**The failure this prevents:** a load that fails quietly leaves the editor on its startup or
template map; a script that "saves the current level" then bakes the arena dressing into the
wrong map and reports success, and every later level made from that template inherits it.

Also:
- An editor save may stage the asset in git when source-control integration is on. Check the
  staged diff (`git diff --cached`) before committing, or another lane's asset rides along.
- A Python callable bound to a component's delegate can be saved into the level as a transient
  reference. Unbind before saving.

## Rendering from a headless run

A commandlet does not draw scene geometry; a capture in one reads back the sky and an empty
base colour. Capture in an editor launched with `-RenderOffscreen` (no window, real rendering),
and **refuse a uniform frame**: a read-back that is not checked calls a black image a success.

A high-resolution screenshot asked for while another is still writing can be dropped silently.
Wait for each file to appear before asking for the next, and report the ones that never did.

## The editor cap and the watchdog

Several off-screen editors share one GPU. Past two or three, editors start to hang on exit:
the log ends with `Log file closed`, the process stays alive, and it holds module DLLs so no
lane can link. Stuck in driver teardown, it may survive `Stop-Process -Force`.

- **Cap concurrent editors** across all lanes (two headless editors is a good default), with
  a simple shared counter or lock file the launch script checks.
- **Watchdog in the launch script:** record the PID at launch; after the script's result file
  appears, give the process a short grace period; if the log ends in `Log file closed` and the
  process is still alive, report it as hung with its PID and the DLLs it holds. Do not wait on
  its exit code.
- A hung editor that cannot be stopped is a report to the developer (it may need a reboot), not a
  reason to kill other processes.
- An out-of-date graphics driver is a suspect; update it before tuning the cap.

## Stopping processes

Stop only the PIDs this session started: capture the PID at launch (`Start-Process -PassThru`)
and target that. The build tool, the editor, `dotnet` and MSBuild are shared with the IDE and
with other lanes. A mutex conflict means something else is working: wait, or ask.

Closing an editor this session launched, to build, is ordinary housekeeping. Closing one the
developer is using is not; the developer says when the editor is theirs.

## A running editor goes stale

Before chasing a visual bug the developer reports from an open editor, ask when that editor was
launched relative to HEAD. An editor left open across a batch of commits shows old DLLs and old
in-memory assets, and its symptoms look exactly like live bugs. A relaunch is cheaper than a
diagnosis.
