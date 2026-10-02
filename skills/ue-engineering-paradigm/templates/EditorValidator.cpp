// Template: registration rules an asset cannot check about itself.

#include "EditorValidator_EnemyRegistration.h"

#include "EnemyDefinition.h"
#include "Engine/AssetManager.h"

#define LOCTEXT_NAMESPACE "EnemyRegistration"

bool UEditorValidator_EnemyRegistration::CanValidateAsset_Implementation(
	const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UEnemyDefinition>();
}

EDataValidationResult UEditorValidator_EnemyRegistration::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UEnemyDefinition* Definition = CastChecked<UEnemyDefinition>(InAsset);

	// Rule 1: the Asset Manager must be able to find it. An enemy saved outside the scanned
	// directories is content that silently never spawns.
	const FPrimaryAssetId Id = Definition->GetPrimaryAssetId();
	if (!UAssetManager::Get().GetPrimaryAssetPath(Id).IsValid())
	{
		AssetFails(InAsset, FText::Format(
			LOCTEXT("NotScanned", "{0} is not registered with the Asset Manager. Move it under a directory scanned for EnemyDefinition (Project Settings > Asset Manager)."),
			FText::FromString(Id.ToString())));
		return EDataValidationResult::Invalid;
	}

	// Add further cross-asset rules here (the class is a Blueprint child of the enemy base,
	// every ability set it grants exists...). One rule, one message, in designer terms.

	// WHY AssetPasses: without it the validator reports that it did not check the asset.
	AssetPasses(InAsset);
	return EDataValidationResult::Valid;
}

#undef LOCTEXT_NAMESPACE
