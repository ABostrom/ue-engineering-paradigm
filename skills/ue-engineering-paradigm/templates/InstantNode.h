// Template: instantaneous nodes on an ability base class.
// Running example: the shared base for Parry, Dodge and Heavy Strike.
//
// WHY plain UFUNCTIONs: no time passes, so no task, no delegates, no lifetime.
// WHY no ActivateAbility override: the Blueprint child of each verb draws the sequence from
// these nodes and the tasks. The C++ base is a machine with no opinions about order.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "MeleeAbility.generated.h"

/** Outcome of resolving a hit. Closed and switched on by C++: an enum, not a tag. */
UENUM(BlueprintType)
enum class EHitOutcome : uint8
{
	Landed,
	Parried
};

UCLASS(Abstract)
class MYGAME_API UMeleeAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * Apply this ability's stagger to Target.
	 * Input is only what the node cannot derive: the instigator is this ability's avatar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Melee|Defense")
	void ApplyStagger(AActor* Target);

	/** Spend stamina from the avatar. Fails activation-side, not here: cost is checked before commit. */
	UFUNCTION(BlueprintCallable, Category = "Melee|Defense")
	void SpendStamina(float Amount);

	/**
	 * Resolve a hit against Target. One exec output per outcome (ExpandEnumAsExecs), so the graph
	 * forks where the author draws different things, with no Break and no Branch.
	 * DamageToApply is final: the parried scale is a property applied here, never arithmetic in a graph.
	 */
	UFUNCTION(BlueprintCallable, Category = "Melee|Strike", meta = (ExpandEnumAsExecs = "Outcome"))
	void ResolveHit(AActor* Target, float BaseDamage, EHitOutcome& Outcome, float& DamageToApply);

protected:
	/** Tuned on each Blueprint child. No meaningful constructor value: IsDataValid checks it. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Strike", meta = (ClampMin = "0", ClampMax = "1"))
	float ParriedDamageScale = 0.f;

	/** The stagger to apply. A Blueprint child assigns it; C++ never names content. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Defense")
	TSubclassOf<class UGameplayEffect> StaggerEffect;
};
