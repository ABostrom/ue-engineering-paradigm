---
id: 09-headless-bake-script
title: A headless bake that does nothing
exercises: skills/ue-agent-lanes/references/headless-editor.md
---

## Prompt

Our agent bakes the fortress arena from the command line:

```
UnrealEditor-Cmd.exe Game.uproject -ExecutePythonScript="bake_arena.py --level /Game/Arenas/L_Fortress" -RenderOffscreen
```

`bake_arena.py` loads the level, calls `generate_local` on the PCG component, then saves "the
current level". The log says generation started and the process exits 0, but the level has no
dressing. Yesterday a variant of the script left the editor running all night. Sometimes, after
a run, the editor stays alive after `Log file closed` and nothing can link until it's gone.
What's going on, and how should the script and the launch work?

## A good answer must

- Explain that `-ExecutePythonScript` requests exit on the next tick after the script returns
  unless keep-alive is set, so latent PCG generation never finishes.
- Recommend `-ExecCmds="py ..."` (or keep-alive) for latent work, with the script itself
  quitting (`quit_editor`) when its work is done, on success and on failure.
- Have the script assert the open level is the target before saving, and refuse otherwise.
- Have the script write a result file the agent reads, rather than trusting the exit code.
- Address the hung-on-exit editor: a cap on concurrent editors and a watchdog that reports the
  PID; stop only that PID.

## A good answer must not

- Suggest a fixed sleep inside the script as the fix.
- Suggest killing every editor process by name.
