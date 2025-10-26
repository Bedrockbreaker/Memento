// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "ConsoleSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MSaveManager.generated.h"

class IMSaveable;
class UMSaveGame;
class UMSaveHistory;
class UMSaveIndex;
class UMSaveNode;
struct FKeyEvent;
struct FMSlotId;

DECLARE_LOG_CATEGORY_EXTERN(LogMSaveManager, Log, All);

#pragma region Delegates

/** Delegate called when a save slot is updated (new node, slot changed, etc.). Passes in the UMSaveGame. */
DECLARE_MULTICAST_DELEGATE_OneParam(FMOnSaveSlotUpdatedDelegate, UMSaveGame*);

/** Delegate called when the save index is updated. Passes in the global UMSaveIndex. */
DECLARE_MULTICAST_DELEGATE_OneParam(FMOnSaveIndexUpdatedDelegate, UMSaveIndex*);

/**
 * Delegate called from AsyncSaveGame. First two parameters are contextual SlotName and UserIndex,
 * third parameter is the resulting UMSaveNode (null on failure).
 */
MEMENTOSAVESYSTEMRUNTIME_API
	DECLARE_DELEGATE_ThreeParams(FMAsyncSaveGameDelegate, const FString&, const int32, UMSaveNode*);

/**
 * Bluprint-exposed delegate called from AsyncSaveGameDynamic. First two paramters are contextual
 * SlotName and UserIndex, third parameter is the resulting UMSaveNode (null on failure).
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncSaveGameDelegateDynamic, const FString&, SlotName, const int32, UserIndex, UMSaveNode*, SaveNode);

/**
 * Delegate called from AsyncLoadGame. First two parameters are contextual SlotName and UserIndex,
 * third parameter is the UMSaveNode that was loaded (null on failure).
 */
MEMENTOSAVESYSTEMRUNTIME_API
	DECLARE_DELEGATE_ThreeParams(FMAsyncLoadGameDelegate, const FString&, const int32, UMSaveNode*);

/**
 * Bluprint-exposed delegate called from AsyncLoadGameDynamic. First two paramters are contextual
 * SlotName and UserIndex, third parameter is the UMSaveNode that was loaded (null on failure).
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncLoadGameDelegateDynamic, const FString&, SlotName, const int32, UserIndex, UMSaveNode*, SaveNode);

/**
 * Delegate called from AsyncRecallGame. First two parameters are contextual SlotName and UserIndex,
 * third parameter is the invisible UMSaveNode automatically created (null on failure).
 */
MEMENTOSAVESYSTEMRUNTIME_API
	DECLARE_DELEGATE_ThreeParams(FMAsyncRecallGameDelegate, const FString&, const int32, UMSaveNode*);

/**
 * Bluprint-exposed delegate called from AsyncRecallGameDynamic. First two paramters are contextual
 * SlotName and UserIndex, third parameter is the invisible UMSaveNode automatically created (null on failure).
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncRecallGameDelegateDynamic, const FString&, SlotName, const int32, UserIndex, UMSaveNode*, SaveNode);

/**
 * Delegate called from AsyncCreateSaveSlot. First two parameters are passed in SlotName and UserIndex,
 * third parameter is the resulting UMSaveGame (null on failure).
 */
MEMENTOSAVESYSTEMRUNTIME_API
	DECLARE_DELEGATE_ThreeParams(FMAsyncCreateSlotDelegate, const FString&, const int32, UMSaveGame*);

/**
 * Bluprint-exposed delegate called from AsyncCreateSaveSlotDynamic. First two paramters are passed in
 * SlotName and UserIndex, third parameter is the resulting UMSaveGame (null on failure).
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncCreateSlotDelegateDynamic, const FString&, SlotName, const int32, UserIndex, UMSaveGame*, SaveGame);

/**
 * Delegate called from AsyncLoadSaveSlot. First two parameters are passed in SlotName and UserIndex,
 * third parameter is the loaded UMSaveGame (null on failure).
 */
MEMENTOSAVESYSTEMRUNTIME_API
	DECLARE_DELEGATE_ThreeParams(FMAsyncLoadSlotDelegate, const FString&, const int32, UMSaveGame*);

/**
 * Bluprint-exposed delegate called from AsyncLoadSaveSlotDynamic. First two paramters are passed in
 * SlotName and UserIndex, third parameter is the loaded UMSaveGame (null on failure).
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncLoadSlotDelegateDynamic, const FString&, SlotName, const int32, UserIndex, UMSaveGame*, SaveGame);

/**
 * Delegate called from AsyncDeleteSaveSlot. First two parameters are passed in SlotName and UserIndex,
 * third parameter is whether the operation succeeded.
 */
MEMENTOSAVESYSTEMRUNTIME_API DECLARE_DELEGATE_ThreeParams(FMAsyncDeleteSlotDelegate, const FString&, const int32, bool);

/**
 * Bluprint-exposed delegate called from AsyncDeleteSaveSlotDynamic. First two paramters are passed in
 * SlotName and UserIndex, third parameter is whether the operation succeeded.
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncDeleteSlotDelegateDynamic, const FString&, SlotName, const int32, UserIndex, bool, bSuccess);

/**
 * Delegate called from AsyncCloneSaveSlot. First two parameters are passed in SlotName and UserIndex for the clone,
 * third parameter is the resulting UMSaveGame clone (null on failure).
 */
MEMENTOSAVESYSTEMRUNTIME_API
	DECLARE_DELEGATE_ThreeParams(FMAsyncCloneSlotDelegate, const FString&, const int32, UMSaveGame*);

/**
 * Bluprint-exposed delegate called from AsyncCloneSaveSlotDynamic. First two paramters are passed in
 * SlotName and UserIndex, third parameter is the resulting UMSaveGame clone (null on failure).
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncCloneSlotDelegateDynamic, const FString&, SlotName, const int32, UserIndex, UMSaveGame*, SaveGame);

#pragma endregion

#pragma region UMSavemanager

/** A game instance subsystem that handles non-linear save and load operations */
UCLASS()
class MEMENTOSAVESYSTEMRUNTIME_API UMSaveManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Called when a save slot is updated (node creation/deletion, save slot change, etc.) */
	FMOnSaveSlotUpdatedDelegate OnSaveSlotUpdated;

	/** Called when the save index is updated (added/removed/cloned save slots, etc.) */
	FMOnSaveIndexUpdatedDelegate OnSaveIndexUpdated;

	/**
	 * Creates a node in the save graph and returns it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMSaveNode* SaveGame(bool bInvisible = false);

	/**
	 * Asynchronously creates a node in the save graph and calls a delegate.
	 */
	void AsyncSaveGame(FMAsyncSaveGameDelegate Delegate, bool bInvisible = false);

	/**
	 * Asynchronously creates a node in the save graph and calls a delegate.
	 *
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Save Game"))
	void AsyncSaveGameDynamic(FMAsyncSaveGameDelegateDynamic Delegate, bool bInvisible = false);

	/**
	 * Loads a node from the save graph.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMSaveNode* LoadGame(FGuid SaveId);

	/**
	 * Asynchronously loads a node from the save graph and calls a delegate.
	 */
	void AsyncLoadGame(FMAsyncLoadGameDelegate Delegate, FGuid SaveId);

	/**
	 * Asynchronously loads a node from the save graph and calls a delegate.
	 *
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Load Game"))
	void AsyncLoadGameDynamic(FMAsyncLoadGameDelegateDynamic Delegate, FGuid SaveId);

	/**
	 * Creates a child save node with the specified parents. The default SequenceParentId is the current save node,
	 * which causes a linear timeline along the sequence parents according to the IRL player.
	 *
	 * Functionally, this will (1) call ISaveable::Load(bRecall = true), then (2) call ISaveable::Save(bRecall = true)
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMSaveNode* RecallGame(FGuid BranchParentId, FGuid SequenceParentId = FGuid(), bool bInvisible = true);

	/**
	 * Asynchronously creates a child save node with the specified parents and calls a delegate. The default
	 * SequenceParentId is the current save node, which causes a linear timeline along the sequence parents according to
	 * the IRL player.
	 *
	 * Functionally, this will:
	 *   1. Call ISaveable::Load(bRecall = true) with the BranchParentId,
	 *   2. Call ISaveable::Save(bRecall = true)
	 */
	void AsyncRecallGame(
		FMAsyncRecallGameDelegate Delegate,
		FGuid					  BranchParentId,
		FGuid					  SequenceParentId = FGuid(),
		bool					  bInvisible = true);

	/**
	 * Asynchronously creates a child save node with the specified parents and calls a delegate. The default
	 * SequenceParentId is the current save node, which causes a linear timeline along the sequence parents according to
	 * the IRL player.
	 *
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Recall Game"))
	void AsyncRecallGameDynamic(
		FMAsyncRecallGameDelegateDynamic Delegate,
		FGuid							 BranchParentId,
		FGuid							 SequenceParentId = FGuid(),
		bool							 bInvisible = true);

	/**
	 * Creates a new save slot with an empty save graph. Returns the new UMSaveGame.
	 * Implicitly deletes any existing save slot with the same name and user index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMSaveGame* CreateSaveSlot(const FString& SlotName, const int32 UserIndex);

	/**
	 * Asynchronously creates a new save slot with an empty save graph and calls a delegate.
	 * Implicitly deletes any existing save slot with the same name and user index.
	 */
	void AsyncCreateSaveSlot(FMAsyncCreateSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex);

	/**
	 * Asynchronously creates a new save slot with an empty save graph and calls a delegate.
	 * Implicitly deletes any existing save slot with the same name and user index.
	 * Bluprint-exposed version.
	 */
	void AsyncCreateSaveSlotDynamic(
		FMAsyncCreateSlotDelegateDynamic Delegate, const FString& SlotName, const int32 UserIndex);

	/**
	 * Loads a save slot. Returns the UMSaveGame.
	 * Passing false for bSetActive will not set the save slot as the active save slot.
	 * Does **NOT** automatically load the most recent save node in the save graph.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMSaveGame* LoadSaveSlot(const FString& SlotName, const int32 UserIndex, bool bSetActive = true);

	/**
	 * Asynchronously loads a save slot and calls a delegate.
	 * Passing false for bSetActive will not set the save slot as the active save slot.
	 * Does **NOT** automatically load the most recent save node in the save graph.
	 */
	void AsyncLoadSaveSlot(
		FMAsyncLoadSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex, bool bSetActive = true);

	/**
	 * Asynchronously loads a save slot and calls a delegate.
	 * Passing false for bSetActive will not set the save slot as the active save slot.
	 * Does **NOT** automatically load the most recent save node in the save graph.
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Load Save Slot"))
	void AsyncLoadSaveSlotDynamic(
		FMAsyncLoadSlotDelegateDynamic Delegate,
		const FString&				   SlotName,
		const int32					   UserIndex,
		bool						   bSetActive = true);

	/**
	 * Loads a save slot if it exists, otherwise creates a new save slot. Returns the UMSaveGame.
	 * Does **NOT** automatically load the most recent save node in the save graph.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMSaveGame* LoadOrCreateSaveSlot(const FString& SlotName, const int32 UserIndex);

	/**
	 * Asynchronously loads a save slot if it exists, otherwise creates a new save slot and calls a delegate.
	 * Does **NOT** automatically load the most recent save node in the save graph.
	 */
	void AsyncLoadOrCreateSaveSlot(FMAsyncLoadSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex);

	/**
	 * Asynchronously loads a save slot if it exists, otherwise creates a new save slot and calls a delegate.
	 * Does **NOT** automatically load the most recent save node in the save graph.
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Load Or Create Save Slot"))
	void AsyncLoadOrCreateSaveSlotDynamic(
		FMAsyncLoadSlotDelegateDynamic Delegate, const FString& SlotName, const int32 UserIndex);

	/** Deletes a save slot and all of its save nodes, returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool DeleteSaveSlot(const FString& SlotName, const int32 UserIndex);

	/**
	 * Asynchronously deletes a save slot and all of its save nodes and calls a delegate.
	 */
	void AsyncDeleteSaveSlot(FMAsyncDeleteSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex);

	/**
	 * Asynchronously deletes a save slot and all of its save nodes and calls a delegate.
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Delete Save Slot"))
	void AsyncDeleteSaveSlotDynamic(
		FMAsyncDeleteSlotDelegateDynamic Delegate, const FString& SlotName, const int32 UserIndex);

	/**
	 * Clones a save slot stored with the given slot name and user index, returns the new UMSaveGame.
	 * Not intended for use in production. Just a debugging aid. VERY SLOW TO RUN.
	 */
	UMSaveGame* CloneSaveSlot(
		const FString& OriginalSlotName,
		const int32	   OriginalUserIndex,
		const FString& NewSlotName,
		const int32	   NewUserIndex);

	/** Returns the index of all known save slots */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Save System")
	TArray<FMSlotId> GetSaveIndex() const;

	/** Returns the currently active save slot */
	UMSaveGame* GetActiveSaveGame() const { return ActiveSaveGame; }

	/** Returns the save history for the currently active save slot */
	UMSaveHistory* GetSaveHistory() const { return SaveHistory; }

	/** Initializes the subsystem, add console commands to manipulate the save manager */
	void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Deinitializes the subsystem, unregister console commands */
	void Deinitialize() override;

private:
	/** The index of all known save slots */
	UPROPERTY()
	TObjectPtr<UMSaveIndex> SaveIndex;

	/** The currently active save slot */
	UPROPERTY()
	TObjectPtr<UMSaveGame> ActiveSaveGame;

	/** Provides query capabilities for the ActiveSaveGame. */
	UPROPERTY()
	TObjectPtr<UMSaveHistory> SaveHistory;

	/** Finds all saveables in the world */
	void FindSaveables(TArray<UObject*>& OutSaveables) const;

	/** Creates a new save node and adds it to the save graph */
	UMSaveNode* CreateSaveNode(FGuid BranchParentId, FGuid SequenceParentId, bool bRecall, bool bInvisible);

	/** Clones a save node. Does not add it to the save graph. */
	UMSaveNode* CloneSaveNode(const UMSaveNode* OriginalSaveNode);

	/** Deserializes a save node and triggers the game to load it */
	bool LoadSaveNode(UMSaveNode* SaveNode, bool bRecall);

	/** Deletes the save nodes within a slot. Does not delete the slot itself. */
	void DeleteSaveGraph(UMSaveGame* SaveGame);
};

#pragma endregion
