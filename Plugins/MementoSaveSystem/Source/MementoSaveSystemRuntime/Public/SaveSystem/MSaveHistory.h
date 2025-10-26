// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "SaveSystem/MSaveData.h"

#include "MSaveHistory.generated.h"

class UMSaveGame;
class UMSaveNode;
// struct FMSaveData;

/** Query-handling object for inspecting historical save data across a UMSaveGame */
UCLASS(BlueprintType)
class MEMENTOSAVESYSTEMRUNTIME_API UMSaveHistory : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * The save game to query. Once provided, it becomes cached for further use.
	 * Can be reinitialized later to avoid allocations.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System|History")
	virtual void Initialize(UMSaveGame* InSaveGame);

	/** Returns the previous save node's data for this saveable. (i.e. from the current save node's sequence parent). */
	UFUNCTION(BlueprintCallable, Category = "Save System|History")
	virtual bool GetLastSaveState(const FString& SaveableId, FMSaveData& OutSaveData) const;

	/** Returns the Nth previous save node's data for this saveable. (i.e. traversing up sequence parents N times). */
	UFUNCTION(BlueprintCallable, Category = "Save System|History")
	virtual bool GetNthLastSaveState(const FString& SaveableId, int32 N, FMSaveData& OutSaveData) const;

	// TODO: Promote FMSaveData to a UObject to allow it be stored in a TArray without ownership
	/** Returns all save states for this saveable across all save nodes. */
	UFUNCTION(BlueprintCallable, Category = "Save System|History")
	virtual bool GetAllSaveStates(const FString& SaveableId, TArray<FMSaveData>& OutSaveData) const;

	/** Returns the number of distinct states for this saveable across all save nodes. */
	UFUNCTION(BlueprintCallable, Category = "Save System|History")
	virtual int32 GetDistinctStateCount(const FString& SaveableId) const;

	/** Returns the number of branch children for this save node. */
	UFUNCTION(BlueprintCallable, Category = "Save System|History")
	virtual int32 GetBranchChildrenCount(const FGuid& SaveNodeId) const;

protected:
	/** The save game to query against. */
	UPROPERTY()
	TObjectPtr<UMSaveGame> SaveGame;

	// TODO: A production-ready system would not store every save node in memory.
	/** Cache of save nodes in memory. */
	UPROPERTY()
	TMap<FGuid, TObjectPtr<UMSaveNode>> SaveNodes;

	/** Helper function to load all save nodes and cache them in memory. */
	virtual void LoadAllNodes();

private:
	FMSaveData DebugSaveData;
};
