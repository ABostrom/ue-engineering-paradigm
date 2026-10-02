# Anti-patterns gallery

Before/after pairs from one melee action game with three verbs: **Parry**, **Dodge** and
**Heavy Strike**.

---

## 1. Type enum plus switch

**Before**
```cpp
UENUM(BlueprintType) enum class EAttackType : uint8 { Light, Heavy, Charged, Spin };

void UGA_MeleeAttack::ActivateAbility(...)
{
    switch (AttackType)
    {
    case EAttackType::Light:   PlayAndHit(LightMontage); break;
    case EAttackType::Charged: StartCharge(); break;          // waits for release
    case EAttackType::Spin:    SpinHitTwiceThenRecover(); break;
    }
}
```
**After** - one Blueprint child per attack, each drawing its own graph from shared nodes:
```
BP_GA_Light:   CommitAbility -> PlayStrike(LightMontage) -> WaitForHitFrame -> ResolveHit -> EndAbility
BP_GA_Charged: WaitForInputRelease -> PlayStrike(ChargedMontage) -> WaitForHitFrame -> ResolveHit -> EndAbility
BP_GA_Spin:    PlayStrike(SpinMontage) -> WaitForHitFrame -> ResolveHit -> WaitForHitFrame -> ResolveHit -> EndAbility
```
**Why:** the enum can only express attacks someone already thought of; Spin needed C++. The
graphs are arrangement, and the fifth attack needs no programmer.

---

## 2. Sequence configured in data

**Before**
```cpp
USTRUCT() struct FAttackStep { GENERATED_BODY() EStepKind Kind; float Duration; int32 Repeat; bool bOnlyIfPreviousHit; };
UPROPERTY(EditDefaultsOnly) TArray<FAttackStep> Steps;   // interpreted by ActivateAbility
```
**After** - the step kinds become nodes; each `Steps` array becomes a graph.

**Why:** `bOnlyIfPreviousHit` is a branch drawn as a checkbox. The next rule ("only if the
target is staggered") is a new field and a new C++ case. A graph already has branches.

---

## 3. Flow in `ActivateAbility`, state in flags

**Before**
```cpp
void UGA_Parry::ActivateAbility(...) { bWindowOpen = true; SetTimer(..., &UGA_Parry::OnWindowClosed); ListenForHit(&UGA_Parry::OnHit); }
void UGA_Parry::OnHit(...)          { if (bWindowOpen) { bParried = true; Stagger(); OpenRiposte(); } else { DrainStamina(); } }
void UGA_Parry::OnWindowClosed()    { bWindowOpen = false; if (!bParried) EndAbility(...); }
```
**After**
```
CommitAbility -> WaitForIncomingHit(Window)
   Parried -> ApplyStagger(Attacker) -> OpenRiposte(RiposteSeconds) -> EndAbility
   Blocked -> SpendStamina(BlockStaminaCost) -> EndAbility
   Expired -> EndAbility
```
**Why:** the flags are a second record of where the sequence is, and nothing forces them to agree
with which handler runs next. In the graph, the wire is the state.

---

## 4. Struct in, struct out, branch, arithmetic

**Before**
```cpp
UFUNCTION(BlueprintCallable) FStrikeResult ResolveHeavyStrikeVsParry(const FStrikeParams& Params, EStrikeSide SideToStagger);
// Graph: Break FStrikeResult -> Branch(bWasParried) -> DamageDealt * 0.5 -> ApplyDamage
```
**After**
```cpp
UENUM(BlueprintType) enum class EHitOutcome : uint8 { Landed, Parried };

UFUNCTION(BlueprintCallable, Category = "Melee", meta = (ExpandEnumAsExecs = "Outcome"))
void ResolveHit(AActor* Target, float BaseDamage, EHitOutcome& Outcome, float& DamageToApply);
// The parried damage scale is a property, applied in C++. Who staggers is the Parry graph's decision.
```
**Why:** verb names in the symbol; opaque struct pins; an enum converted back into an actor; a
result the author must re-derive; arithmetic in the graph.

---

## 5. Content path in C++

**Before**
```cpp
UWeaponComponent::UWeaponComponent()
{
    static ConstructorHelpers::FObjectFinder<UAnimMontage> Parry(TEXT("/Game/Anims/AM_Parry.AM_Parry"));
    ParryMontage = Parry.Object;
}
```
**After**
```cpp
UPROPERTY(EditDefaultsOnly, Category = "Parry") TObjectPtr<UAnimMontage> ParryMontage;   // set on BP_WeaponComponent
```
**Why:** content decided in code, breaks on move/rename, and hides a missing assignment.

---

## 6. Fallback for missing data

**Before**
```cpp
if (!WeaponDefinition) { WeaponDefinition = LoadObject<UWeaponDefinition>(nullptr, TEXT("/Game/Weapons/DA_DefaultSword")); }
if (!WeaponDefinition->HitMontage) { ApplyDamageNow(); return; }
```
**After**
```cpp
// Authoring time
EDataValidationResult UWeaponDefinition::IsDataValid(FDataValidationContext& Context) const { /* HitMontage required */ }

// The edge: refuse, loudly, once.
bool UWeaponComponent::Equip(const UWeaponDefinition* Definition)
{
    if (!Definition) { UE_LOG(LogWeapons, Error, TEXT("%s: Equip called with no definition"), *GetNameSafe(GetOwner())); return false; }
    ...
}
// Inside: trust, with an invariant.
void UWeaponComponent::PlayHit() { check(Equipped); ... }
```
**Why:** a fallback turns broken content into content that looks fine.

---

## 7. Hand list of content

**Before**
```cpp
UWaveDirector::UWaveDirector() { AllowedTypes = { EEnemyType::Grunt, EEnemyType::Archer, EEnemyType::Brute }; }
```
**After**
```cpp
TArray<FPrimaryAssetId> Ids;
UAssetManager::Get().GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("EnemyDefinition")), Ids);
// ...or the wave's roster is a data asset a designer edits.
```
**Why:** a second copy of a fact; a new enemy is invisible until a programmer edits the list.

---

## 8. Data-only C++ subclass

**Before**
```cpp
UCLASS() class UGA_ChargedHeavyStrike : public UGA_HeavyStrike
{ GENERATED_BODY() public: UGA_ChargedHeavyStrike() { WindupSeconds = 1.2f; DamageMultiplier = 2.5f; } };
```
**After** - `BP_GA_ChargedHeavyStrike`, a Blueprint child of the same base, with the values set
as defaults. Delete the C++ class.

---

## 9. Empty Blueprint wrapper

**Before** - `BP_GA_Dodge` sets `DodgeMontage` and `IFrameSeconds`; its event graph is empty;
`UGA_Dodge::ActivateAbility` does everything.

**After** - `UGA_Dodge` no longer overrides `ActivateAbility`; `BP_GA_Dodge` draws
`CommitAbility -> GrantInvulnerability(IFrameSeconds) -> PlayDodge -> WaitForMontageEnd -> EndAbility`.

---

## 10. Hidden wait in a lambda

**Before**
```cpp
Montage->OnCompleted.AddLambda([this]{ GetWorld()->GetTimerManager().SetTimer(H, [this]{ OpenRiposte(); }, 0.1f, false); });
```
**After** - `WaitForMontageEnd -> WaitForDelay(RiposteDelay) -> OpenRiposte`, each a node the
author can see, branch on and cancel.

---

## 11. A rule in an overridable container

**Before**
```cpp
UMeleeAbility::UMeleeAbility() { ActivationBlockedTags.AddTag(TAG_State_Paused); }  // never reaches older BP children
```
**After**
```cpp
bool UMeleeAbility::CanActivateAbility(...) const
{
    if (IsGamePaused(ActorInfo)) { return false; }  // a rule for every subclass lives in code
    return Super::CanActivateAbility(...);
}
```

---

## 12. Second store of a replicated fact

**Before** - `AArenaGameMode::Kills` (a `TMap`) plus `AArenaPlayerState::Kills`; the HUD calls
`GetWorld()->GetAuthGameMode<AArenaGameMode>()->GetKills(PS)` (null on every client).

**After** - `AArenaPlayerState::Kills` (replicated) is the one store; the GameMode writes it; the
HUD reads it; a "leader" is a derived query.

---

## 13. World-type list instead of asking the dependency

**Before**
```cpp
if (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) { World->GetSubsystem<UArenaRegistry>()->Register(this); }
```
**After**
```cpp
if (UArenaRegistry* Registry = World->GetSubsystem<UArenaRegistry>()) { Registry->Register(this); }
```

---

## 14. Unreplicated composed component

**Before** - the server adds `UParryTrackerComponent` at match start; it replicates nothing, so
`SetIsReplicatedByDefault` was left false. Abilities it grants name it as `SourceObject`, and on
clients that reference is null.

**After** - `SetIsReplicatedByDefault(true)` in its constructor. What travels is the object's
identity, not its data.

---

## 15. A tag for a closed set

**Before** - `Cost.Stamina`, `Cost.Poise`, `Cost.Health` tags that no C++ reads, while
`EAbilityCost` already exists and is what the code switches on.

**After** - delete the tags; the enum is the model. Keep tags for open, design-driven sets
(factions, damage flavours designers invent).
