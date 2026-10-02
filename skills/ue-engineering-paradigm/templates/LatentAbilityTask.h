// Template: a latent node. One UAbilityTask per thing a verb WAITS on.
// Running example: Parry waits for an incoming hit inside a timing window.
//
// WHY a task: time passes here. Instantaneous work (stagger, stamina, riposte) is a plain
// BlueprintCallable on the ability - see InstantNode.h. Choosing wrong either way is how a
// node library goes bad.
//
// WHY one delegate per outcome: each BlueprintAssignable delegate becomes an exec pin, so the
// graph forks exactly where the author draws different things. No result struct, no Switch node.

#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "GameplayTagContainer.h"
#include "AbilityTask_WaitForIncomingHit.generated.h"

struct FGameplayEventData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIncomingHitDelegate, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FIncomingHitExpiredDelegate);

UCLASS()
class MYGAME_API UAbilityTask_WaitForIncomingHit : public UAbilityTask
{
	GENERATED_BODY()

public:
	/** The hit arrived inside the window. Attacker is only meaningful here, so it only exists here. */
	UPROPERTY(BlueprintAssignable)
	FIncomingHitDelegate Parried;

	/** The hit arrived after the window closed. */
	UPROPERTY(BlueprintAssignable)
	FIncomingHitDelegate Blocked;

	/** Nothing hit us before MaxWaitSeconds. */
	UPROPERTY(BlueprintAssignable)
	FIncomingHitExpiredDelegate Expired;

	/**
	 * Wait for an incoming hit and judge it against the parry window.
	 * Named for what the verb waits on (WaitFor<What>), verb-first, no narration.
	 * Inputs are only what the task cannot derive: the owning ability is hidden and defaulted to self.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks|Melee",
		meta = (DisplayName = "Wait For Incoming Hit", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_WaitForIncomingHit* WaitForIncomingHit(UGameplayAbility* OwningAbility, FGameplayTag HitEventTag, float ParryWindowSeconds, float MaxWaitSeconds);

	virtual void Activate() override;

protected:
	// WHY OnDestroy: it runs however the task ends (outcome, ability cancelled, ability ended),
	// so cleanup cannot be skipped by a branch. This is the Blueprint-side RAII.
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	void HandleHitEvent(const FGameplayEventData* Payload);
	void HandleTimeout();

	FGameplayTag HitEventTag;
	float ParryWindowSeconds = 0.f;
	float MaxWaitSeconds = 0.f;
	double WindowOpenedAt = 0.0;

	FDelegateHandle HitEventHandle;
	FTimerHandle TimeoutHandle;
};
