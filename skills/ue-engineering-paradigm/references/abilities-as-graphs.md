# GAS abilities as Blueprint chains of C++ nodes

**The graph holds the order. C++ holds everything else.**

Anything that is *sequence* - what happens next, what happens if it worked, what happens if it
did not - goes in the ability's Blueprint graph, because that is what a graph is a picture of.
Anything that is *computation* - resolving a hit, judging a timing window, deciding whether an
activation is allowed - stays in C++, where it is tested and the compiler has an opinion. A
graph doing arithmetic is a `.cpp` somebody drew instead of wrote.

## Why the sequence belongs in the graph

A `UAbilityTask` cannot block. So in C++ every "then" becomes another function and every
"meanwhile" becomes another member. Parry - press, wait for a hit inside the window, branch on
parried or blocked, open a riposte or spend stamina - written in C++ is an `ActivateAbility`,
three callbacks, an `OnWindowExpired`, and `bWindowOpen`, `bParried`, `bRiposteOpen`. Each new
branch adds a handler *and* a flag, and nothing forces the flags to agree with which handler may
run next.

In a graph the wire is the state. The author cannot reach "open riposte" without having come
out of the `Parried` pin, so being at the wrong point in the sequence stops being possible
rather than being caught.

## What Parry looks like

```
Event ActivateAbility
  -> CommitAbility
  -> WaitForIncomingHit (Window = ParryWindowSeconds)
        Parried ->  ApplyStagger(Target = Attacker)
                    -> OpenRiposte(Duration = RiposteSeconds)
                    -> EndAbility
        Blocked ->  SpendStamina(Amount = BlockStaminaCost)
                    -> EndAbility
        Expired ->  EndAbility
```

- `UMeleeAbility` (C++) provides `ApplyStagger`, `OpenRiposte`, `SpendStamina` and the base
  class's shared behaviour. It does **not** override `ActivateAbility`.
- `UAbilityTask_WaitForIncomingHit` (C++) listens for the hit, judges the window (computation,
  in C++), and fires exactly one of `Parried` / `Blocked` / `Expired`.
- `BP_GA_Parry` (Blueprint child) draws the graph above and owns `ParryWindowSeconds`,
  `RiposteSeconds`, `BlockStaminaCost`, the montage and the cue tags as `EditDefaultsOnly`
  properties.
- `BP_GA_PerfectParry` - when designers want it - is another child with a narrower window and
  a different Parried branch. No enum, no flag, no programmer.

Dodge is `CommitAbility -> PlayDodge -> WaitForHitFrame(Recovery) -> EndAbility`. Heavy Strike
is `WaitForInputRelease -> PlayStrike -> WaitForHitFrame -> ResolveHit -> ...`. Three verbs,
one small node set.

## A node is a whole verb

Size each node as **one phrase of the sentence**: something an onlooker could say happened
("he parried it", "he staggered him"), not a step toward something happening ("he lines up
the preconditions"). This is what makes "no arithmetic in graphs" enforceable: a node set cut
too fine leaves the author wiring the joins, and **the joins are where the maths goes**. Fine
nodes are an invitation to draw a `.cpp`.

That test **sizes** a node. It does **not** name one - see the naming rules in
`node-contract.md`.

## What "done" means

Measure it per ability, not per codebase:

1. Does the Blueprint child's graph hold the sequence, or is it an empty wrapper over a C++
   `ActivateAbility`?
2. Could someone author the *next* verb in Blueprint alone, with no missing node sending them
   back to C++?

If either answer is no, the conversion is not done, however clean the C++ looks.

## The cost, and how to pay it

The order of events moves into binary assets a compiler cannot check, and graph merges are
unpleasant. Pay for it by keeping every node in C++ and tested **at the outcome level**, so a
miswired graph fails loudly and obviously in play instead of quietly corrupting state. That is
a design requirement, not a hope. Keep graphs short (a whole-verb node set does this for you),
and give each verb its own asset so two people rarely edit the same graph.

## Smells and their conversions

### 1. Flow in `ActivateAbility`

```cpp
void UGA_Parry::ActivateAbility(...)
{
    CommitAbility(...);
    bWindowOpen = true;
    GetWorld()->GetTimerManager().SetTimer(WindowTimer, this, &UGA_Parry::OnWindowClosed, ParryWindow);
    HitDelegateHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(HitTag).AddUObject(this, &UGA_Parry::OnHit);
}
void UGA_Parry::OnHit(const FGameplayEventData* Data) { if (bWindowOpen) { bParried = true; ... } else { ... } }
void UGA_Parry::OnWindowClosed() { bWindowOpen = false; if (!bParried) { ... } }
```

**Convert:** the timer, the listener and the window judgement become one task
(`WaitForIncomingHit`) whose outputs are the branches. Delete the flags. The ability base
stops overriding `ActivateAbility`; the Blueprint child draws it.

### 2. A flag sequence that should be a wait

`bTargetLocked`, `bAwaitingSecondClick`, `LockedTarget` - members that record *where in the
sequence we are*. Every one of them is a wait the graph should show:
`WaitForTargetSelection -> WaitForConfirm`. If a flag answers "have we got to step N yet?", it is
a second source of truth about the graph's own position. Delete it when the wait exists.

Related: a `CanStillCancel()` guard. Cancel belongs to the task that can be cancelled - the task
that is waiting binds the cancel input and fires `Cancelled`; a task that cannot be cancelled
never binds it. Gating falls out of which task is running.

### 3. Hidden latent flow in a lambda

```cpp
Task->OnFinished.AddLambda([this]() { GetWorld()->GetTimerManager().SetTimerForNextTick([this]() { FinishStrike(); }); });
```

A wait the graph cannot see. **Convert:** a named task (`WaitForStrikeRecovery`) the graph
wires, so the author sees that time passes and can branch on how it ended.

### 4. Data-only subclasses

```cpp
UGA_HeavyStrike::UGA_HeavyStrike() { DamageMultiplier = 2.f; WindupSeconds = 0.6f; }
class UGA_ChargedHeavyStrike : public UGA_HeavyStrike { /* constructor only */ };
```

A C++ class whose only content is a constructor is data pretending to be a class. **Convert:**
delete the class; the values move to the Blueprint child (or a data asset). If two variants
differ only in a value, they are two Blueprint children of the same base.

### 5. Tunables in constructors

Any number a designer might want to change (`ParryWindowSeconds`, `TargetRange`, stamina
costs) set in a C++ constructor is a recompile for a tuning pass. **Convert:**
`UPROPERTY(EditDefaultsOnly, Category = "Parry")` with no meaningful constructor value, set on
the Blueprint child, and checked by `IsDataValid` if zero is invalid. After moving a value, open
and resave each Blueprint child (see `traps.md`: containers and defaults freeze on save).

### 6. Empty Blueprint wrappers

A Blueprint child that sets a montage and a data asset over a C++ `ActivateAbility` is not a
Blueprint ability. **Convert:** move the sequence into its graph, one smell at a time (1 to 3
above), and delete the C++ override when the graph draws everything.

### 7. One ability, a type enum, a switch

See `anti-patterns-gallery.md` #1. **Convert:** one Blueprint child per case, each drawing its
own graph from the shared nodes. Then delete the enum.

### 8. A step list in data

`TArray<TInstancedStruct<FAttackStep>> Steps` interpreted by a C++ `ActivateAbility`. This is a
graph engine somebody wrote instead of using the one in the editor. Every new step kind and
every ordering rule ("only if the first hit landed") is C++ again. **Convert:** the step kinds
become nodes; each list becomes a graph.

## Conversion order

1. Add the nodes the graph will need (tasks for the waits, functions for the instants), each
   tested at the outcome level.
2. Draw the graph in the Blueprint child, compile it, and run the verb in play.
3. Delete the C++ `ActivateAbility` override, the callbacks and the flags.
4. Move tunables onto the Blueprint child; resave.
5. Collapse data-only subclasses into Blueprint children.

Do one verb end to end before starting the next, and keep the game playable after each verb.
Never rename a node's factory while graphs still call it (`traps.md`).

## Rules that make verbs final and simple

- **A committed action is final.** If a verb, once committed, cannot be re-targeted, its graph
  is a straight line. Refuse a second activation at `CanActivateAbility` (server side, not only
  on the client) instead of handling "changed destination" inside the graph.
- **Activation gates are code, per-verb entries are data.** A rule every verb must obey
  (abilities are blocked while the match is paused) goes in the gate function in C++, not in a
  tag container that Blueprint children can override. Per-verb blocked tags stay in the
  container, where the author can see them. See `traps.md`.
