// Template: discover content by asset, never by hand list; load it asynchronously.
//
// Setup (data, not code): Project Settings > Asset Manager > Primary Asset Types To Scan:
//   Primary Asset Type = EnemyDefinition, Asset Base Class = EnemyDefinition,
//   Directories = the folder(s) designers put DA_Enemy_* in.
// Acceptance test: author one new DA_Enemy_* in that folder. It must appear in play with no
// rebuild, no code edit and no config edit.

#include "EnemyRoster.h"   // a UWorldSubsystem or component declaring the members used below

#include "EnemyDefinition.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UEnemyRoster::LoadAllEnemies()
{
	UAssetManager& AssetManager = UAssetManager::Get();

	// Query without loading: the Asset Manager already knows every enemy that exists.
	TArray<FPrimaryAssetId> EnemyIds;
	AssetManager.GetPrimaryAssetIdList(UEnemyDefinition::AssetType, EnemyIds);

	// Load the definitions plus everything tagged AssetBundles="Gameplay" on them, in one batch,
	// behind a loading screen. Keep the handle: it keeps the request alive and can be cancelled.
	LoadHandle = AssetManager.LoadPrimaryAssets(
		EnemyIds,
		{ FName(TEXT("Gameplay")) },
		FStreamableDelegate::CreateUObject(this, &UEnemyRoster::HandleEnemiesLoaded, EnemyIds));
	// Documented behaviour: if there is no work to do, LoadHandle is null and the delegate is
	// still called - so do not treat a null handle as failure.
}

void UEnemyRoster::HandleEnemiesLoaded(TArray<FPrimaryAssetId> EnemyIds)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	for (const FPrimaryAssetId& Id : EnemyIds)
	{
		UEnemyDefinition* Definition = AssetManager.GetPrimaryAssetObject<UEnemyDefinition>(Id);

		// The edge: refuse loudly, once. IsDataValid caught authoring errors in the editor; this
		// catches anything that reached a build anyway. No fallback enemy.
		if (!Definition || Definition->EnemyClass.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("Enemy %s failed to load or has no EnemyClass; it will not spawn."), *Id.ToString());
			continue;
		}
		Available.Add(Definition);   // past this point, code trusts Definition
	}
}

// Filtering by kind without loading: mark a field AssetRegistrySearchable and read it from
// FAssetData via UAssetManager::GetPrimaryAssetDataList(UEnemyDefinition::AssetType, ...).
