// Template: validation at the edge. Bad authored data is an editor error, never a runtime fallback.

#include "EnemyDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "EnemyDefinition"

const FPrimaryAssetType UEnemyDefinition::AssetType(TEXT("EnemyDefinition"));

FPrimaryAssetId UEnemyDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}

#if WITH_EDITOR
EDataValidationResult UEnemyDefinition::IsDataValid(FDataValidationContext& Context) const
{
	// WHY: compiling a Blueprint runs IsDataValid on its class default object. The CDO holds none
	// of the data an instance holds, so a strict check here would compile any Blueprint subclass
	// into an error state. Validate the data, not the empty shape.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return EDataValidationResult::NotValidated;
	}

	EDataValidationResult Result = Super::IsDataValid(Context);

	if (EnemyClass.IsNull())
	{
		Context.AddError(LOCTEXT("NoClass", "EnemyClass is not set. Assign the enemy's Blueprint class."));
		Result = EDataValidationResult::Invalid;
	}

	if (!Faction.IsValid())
	{
		Context.AddError(LOCTEXT("NoFaction", "Faction is not set. Pick a Faction.* tag."));
		Result = EDataValidationResult::Invalid;
	}

	const bool bHasStats = Fragments.ContainsByPredicate([](const TInstancedStruct<FEnemyFragment>& Fragment)
	{
		return Fragment.GetPtr<FEnemyFragment_Stats>() != nullptr;
	});
	if (!bHasStats)
	{
		Context.AddError(LOCTEXT("NoStats", "Every enemy needs a Stats fragment."));
		Result = EDataValidationResult::Invalid;
	}

	// Name what is wrong, in designer terms. No "repair", no default: the author fixes the asset.
	return Result == EDataValidationResult::NotValidated ? EDataValidationResult::Valid : Result;
}
#endif

#undef LOCTEXT_NAMESPACE
