// Template: implementation of a latent node. See LatentAbilityTask.h.

#include "AbilityTask_WaitForIncomingHit.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAbilityTask_WaitForIncomingHit* UAbilityTask_WaitForIncomingHit::WaitForIncomingHit(
	UGameplayAbility* OwningAbility, FGameplayTag InHitEventTag, float InParryWindowSeconds, float InMaxWaitSeconds)
{
	// The factory only creates and stores inputs. It must NOT start work or fire delegates:
	// the Blueprint node calls ReadyForActivation() (and so Activate()) after binding the outputs.
	// From C++, call Task->ReadyForActivation() yourself after binding.
	UAbilityTask_WaitForIncomingHit* Task = NewAbilityTask<UAbilityTask_WaitForIncomingHit>(OwningAbility);
	Task->HitEventTag = InHitEventTag;
	Task->ParryWindowSeconds = InParryWindowSeconds;
	Task->MaxWaitSeconds = InMaxWaitSeconds;
	return Task;
}

void UAbilityTask_WaitForIncomingHit::Activate()
{
	// Validated at the edge: the ability's IsDataValid guarantees a positive window, so this is an
	// invariant, not a case to handle.
	ensureMsgf(ParryWindowSeconds > 0.f, TEXT("%s: ParryWindowSeconds must be positive"), *GetNameSafe(Ability));

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	check(ASC);

	WindowOpenedAt = GetWorld()->GetTimeSeconds();
	HitEventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(HitEventTag)
		.AddUObject(this, &UAbilityTask_WaitForIncomingHit::HandleHitEvent);

	GetWorld()->GetTimerManager().SetTimer(TimeoutHandle, this, &UAbilityTask_WaitForIncomingHit::HandleTimeout, MaxWaitSeconds, false);

	Super::Activate();
}

void UAbilityTask_WaitForIncomingHit::HandleHitEvent(const FGameplayEventData* Payload)
{
	// The judgement is computation, so it lives here in C++, not in the graph.
	AActor* Attacker = Payload ? const_cast<AActor*>(Payload->Instigator.Get()) : nullptr;
	const bool bInsideWindow = (GetWorld()->GetTimeSeconds() - WindowOpenedAt) <= ParryWindowSeconds;

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		(bInsideWindow ? Parried : Blocked).Broadcast(Attacker);
	}
	EndTask();
}

void UAbilityTask_WaitForIncomingHit::HandleTimeout()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Expired.Broadcast();
	}
	EndTask();
}

void UAbilityTask_WaitForIncomingHit::OnDestroy(bool bInOwnerFinished)
{
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(HitEventTag).Remove(HitEventHandle);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimeoutHandle);
	}
	Super::OnDestroy(bInOwnerFinished);
}
