// Template: an extension seam a Blueprint child can override.
//
// WHY not `protected virtual`: a Blueprint child cannot override a C++ virtual, so extending it
// needs a programmer. A BlueprintNativeEvent has a C++ default (written as <Name>_Implementation)
// and the generated thunk calls the Blueprint override when a child provides one.
//
// Use a seam for a genuine per-verb difference in COMPUTATION. Do not use it to let a Blueprint
// re-implement the sequence its graph already draws.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "StrikeAbility.generated.h"

UCLASS(Abstract)
class MYGAME_API UStrikeAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * Poise damage this strike deals to Target. Heavy Strike's child may scale by charge time;
	 * a Light child keeps the default. Callers (C++ or graph) always call CalculatePoiseDamage(),
	 * never the _Implementation directly.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Melee|Strike")
	float CalculatePoiseDamage(const AActor* Target) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Strike")
	float BasePoiseDamage = 0.f;
};

// --- StrikeAbility.cpp ---
//
// float UStrikeAbility::CalculatePoiseDamage_Implementation(const AActor* Target) const
// {
//     return BasePoiseDamage;   // the default every child gets unless it overrides the event
// }
