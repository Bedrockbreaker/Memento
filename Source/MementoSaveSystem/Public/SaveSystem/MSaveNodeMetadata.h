// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "MSaveNodeMetadata.generated.h"

USTRUCT(BlueprintType)
struct MEMENTOSAVESYSTEM_API FMSaveNodeMetadata
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FGuid SaveId;

	UPROPERTY(BlueprintReadOnly)
	FGuid BranchParentId;

	UPROPERTY(BlueprintReadOnly)
	FGuid SequenceParentId;

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	bool bInvisible = false;

	// TODO: move these functions to the SaveManager for blueprint-friendly access

	// UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	// FMemSaveNodeMetadata* GetBranchParent() const;

	// UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	// FMemSaveNodeMetadata* GetSequenceParent() const;

	// UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveSystem")
	// bool IsRoot() const { return !BranchParentId.IsValid() && !SequenceParentId.IsValid(); }
};
