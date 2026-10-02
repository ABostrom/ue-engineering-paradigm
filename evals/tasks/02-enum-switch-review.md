---
id: 02-enum-switch-review
title: Review an enum-plus-switch ability
exercises: SKILL.md (one BP child per verb, red flags), references/anti-patterns-gallery.md, references/reviewing.md
---

## Prompt

You're reviewing a pull request in a UE5 melee game that uses GAS. It adds a fifth
attack type. The code:

```cpp
UENUM(BlueprintType)
enum class EAttackType : uint8 { Light, Heavy, Charged, Plunging, Spin /* new */ };

UCLASS()
class UGA_MeleeAttack : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) EAttackType AttackType = EAttackType::Light;
    UPROPERTY(EditDefaultsOnly) float WindupSeconds = 0.2f;
    UPROPERTY(EditDefaultsOnly) float DamageMultiplier = 1.f;

    virtual void ActivateAbility(...) override
    {
        switch (AttackType)
        {
        case EAttackType::Light:    PlayAndHit(LightMontage); break;
        case EAttackType::Heavy:    PlayAndHit(HeavyMontage); break;
        case EAttackType::Charged:  StartCharge(); break;      // waits for release, then hits
        case EAttackType::Plunging: CheckAirborneThenHit(); break;
        case EAttackType::Spin:     SpinHitTwiceThenRecover(); break; // new
        }
    }
};
```

Five data-only Blueprints (`BP_GA_Light`, ... `BP_GA_Spin`) each pick an `AttackType` in
the dropdown. The author says: "Adding attacks is easy now, it's just a new enum value."

Write your review. Approve, request changes, or something else? What would you have them
do, and in what order?

## A good answer must

- Name the problem as the **type-enum-plus-switch / config-schema trap**: the enum can
  only express attacks someone already thought of, and each new one (Spin needs two
  hits) needed **C++ edits**, not arrangement.
- Recommend **one Blueprint child per attack with its own graph**, sharing a C++ base
  that exposes whole-verb nodes (play the windup, wait for the hit frame, apply the hit,
  wait for release...).
- Apply the **falsifiable test**: the next attack should be authorable in Blueprint alone.
- Warn about the **migration hazard**: changing or removing a `UFUNCTION`/enum that
  graphs use breaks Blueprints silently; land the C++ and graph edits together (or use
  CoreRedirects) and recompile every affected Blueprint.

## A good answer must not

- Approve the PR as "just a new enum value".
- Propose a richer config struct or DataTable of `{windup, hitCount, requiresAirborne}`
  fields as the fix for *sequence* differences (that is the same trap, configured).
- Propose moving the switch into a Blueprint `Switch on EAttackType` node.
