// Template: a primary data asset that validates itself.
// Running example: one UEnemyDefinition asset per enemy (DA_Enemy_Ghoul, DA_Enemy_Revenant...).
//
// WHY one asset per enemy: adding an enemy is adding a file. No enum, no hand list, no shared
// table everyone edits. The Asset Manager finds it (see DefinitionDiscovery.cpp).

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "EnemyDefinition.generated.h"

class AEnemyCharacter;

/** Base for composable aspects of an enemy. Systems query the fragment types they understand. */
USTRUCT(BlueprintType)
struct FEnemyFragment
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType, meta = (DisplayName = "Stats"))
struct FEnemyFragment_Stats : public FEnemyFragment
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "1"))
	float MaxHealth = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0"))
	float MaxPoise = 0.f;
};

UCLASS(BlueprintType)
class MYGAME_API UEnemyDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** The Blueprint child to spawn. Soft, so listing enemies does not load them. Bundled for play. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<AEnemyCharacter> EnemyClass;

	/** Factions are an open, designer-driven set: a tag, not an enum. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy", meta = (Categories = "Faction"))
	FGameplayTag Faction;

	/** Aspects, never steps. A fragment describes what the enemy IS, not what happens next. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FEnemyFragment>> Fragments;

	/** Pin the primary asset type so it does not depend on the class hierarchy. */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	static const FPrimaryAssetType AssetType;
};
