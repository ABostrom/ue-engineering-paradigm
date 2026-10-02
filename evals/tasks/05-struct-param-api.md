---
id: 05-struct-param-api
title: Review a BlueprintCallable API with a struct param
exercises: references/node-contract.md, SKILL.md (verb name inside a C++ symbol)
---

## Prompt

Review this Blueprint-facing API from a UE5 melee game's GAS ability base class. It is
called from the Heavy Strike ability's Blueprint graph.

```cpp
USTRUCT(BlueprintType)
struct FStrikeParams
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite) AActor* Attacker = nullptr;
    UPROPERTY(BlueprintReadWrite) AActor* Defender = nullptr;
    UPROPERTY(BlueprintReadWrite) float BaseDamage = 0.f;
    UPROPERTY(BlueprintReadWrite) bool bIsCharged = false;
};

UENUM(BlueprintType)
enum class EStrikeSide : uint8 { Instigator, Target };

USTRUCT(BlueprintType)
struct FStrikeResult
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) bool bWasParried = false;
    UPROPERTY(BlueprintReadOnly) float DamageDealt = 0.f;
    UPROPERTY(BlueprintReadOnly) AActor* Staggered = nullptr; // null unless parried
};

UFUNCTION(BlueprintCallable, Category="Melee")
FStrikeResult ResolveHeavyStrikeVsParry(const FStrikeParams& Params, EStrikeSide SideToStagger);
```

The graph calls it, breaks the result, branches on `bWasParried`, and on the true side
multiplies `DamageDealt` by 0.5 before applying it. What would you change, and why?

## A good answer must

- Flag the **verb names inside the C++ symbol** ("HeavyStrike", "Parry") as a sentence
  living in C++ that a graph should hold; propose general nodes the graph arranges.
- Replace the **opaque struct input** with **flat pins** the author can see and wire;
  drop inputs the node can derive (the Attacker is the ability's avatar).
- Replace the **enum that is converted back into an actor** with passing the actor.
- Replace the result struct + Branch with **exec outputs**, only where there are two
  different things to draw, and **no null-half-the-time output**.
- Move the **0.5 multiplier out of the graph** (arithmetic belongs in C++ or a tunable).
- Mention naming as public API: `<Verb><Object>` verb-first, and that renaming later
  orphans graphs (so get it right, or use redirects).

## A good answer must not

- Suggest only cosmetic changes (Category, `BlueprintPure`, `DisplayName`, comments).
- Recommend adding more fields to `FStrikeParams` for future strike types.
