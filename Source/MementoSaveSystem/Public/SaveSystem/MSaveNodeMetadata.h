// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "MSaveNodeMetadata.generated.h"

/** Lightweight metadata for a single save node. Used to avoid unncessary deserialization. */
USTRUCT(BlueprintType)
struct MEMENTOSAVESYSTEM_API FMSaveNodeMetadata
{
	GENERATED_BODY()

public:
	/** The id of the corresponding save node. */
	UPROPERTY(BlueprintReadOnly)
	FGuid SaveId;

	/** The id of the branch parent node (chronological parent). */
	UPROPERTY(BlueprintReadOnly)
	FGuid BranchParentId;

	/** The id of the sequence parent node (logical parent). */
	UPROPERTY(BlueprintReadOnly)
	FGuid SequenceParentId;

	/** The timestamp of when the save node was created. */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Whether the node is invisible or not (hidden in the UI). */
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
