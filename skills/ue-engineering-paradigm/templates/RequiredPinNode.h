// Template: make the unsafe graph undrawable instead of detected at runtime.
//
// Two shapes the Blueprint compiler rejects when left unwired on a function-call node:
//
//  UPARAM(Required)  sets CPF_RequiredParm: "Parameter must be linked explicitly in blueprint.
//                    Leaving the parameter out results in a compile error." (ObjectMacros.h;
//                    reported by UK2Node_CallFunction::ValidateRequiredPins).
//
//  UPARAM(ref)       a by-reference input. Without AutoCreateRefTerm meta for it, an unwired
//                    by-ref pin fails with "must have an input wired into it ("by ref" params
//                    expect a valid input to operate on)" (UEdGraphSchema_K2::IsPinDefaultValid).
//
// Running example: a riposte opening is a token. Whoever opens it must close or spend it; leaking
// it would leave the defender's riposte state stuck open.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "RiposteLibrary.generated.h"

/** A riposte opening. Travels on a wire from OpenRiposte to SpendRiposte / CloseRiposte. */
USTRUCT(BlueprintType)
struct FRiposteOpening
{
	GENERATED_BODY()

	// Deliberately not BlueprintReadWrite: authors pass the token, they do not build or edit one.
	UPROPERTY()
	TWeakObjectPtr<AActor> Defender;

	UPROPERTY()
	double OpenedAt = 0.0;
};

UCLASS()
class MYGAME_API URiposteLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Open a riposte for Defender. The returned token must reach Spend or Close. */
	UFUNCTION(BlueprintCallable, Category = "Melee|Riposte")
	static FRiposteOpening OpenRiposte(UPARAM(Required) AActor* Defender, float DurationSeconds);

	/**
	 * Spend the opening on a counter-attack. `ref` means a graph cannot call this without wiring
	 * a real opening into it - there is no default to fall back to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Melee|Riposte")
	static void SpendRiposte(UPARAM(ref) FRiposteOpening& Opening, UPARAM(Required) AActor* Target);

	/** Close the opening unused. */
	UFUNCTION(BlueprintCallable, Category = "Melee|Riposte")
	static void CloseRiposte(UPARAM(ref) FRiposteOpening& Opening);
};

// Note: these checks are made on function-call nodes. Verify on your engine version before
// relying on them for a latent task's factory pins.
