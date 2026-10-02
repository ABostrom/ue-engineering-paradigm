// Template: a project-wide editor validator (editor module only; depends on "DataValidation").
//
// WHY a validator instead of IsDataValid: this rule is about REGISTRATION across assets - content
// that exists but would never be found. That failure has no error of its own; it just never shows
// up in play. A validator turns "never shows up" into a red line in the editor.
//
// UEditorValidatorBase subclasses (C++ and Blueprint) are gathered on editor start.

#pragma once

#include "EditorValidatorBase.h"
#include "EditorValidator_EnemyRegistration.generated.h"

UCLASS()
class MYGAMEEDITOR_API UEditorValidator_EnemyRegistration : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};
