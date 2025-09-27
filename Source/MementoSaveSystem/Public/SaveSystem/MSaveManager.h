// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "ConsoleSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MSaveManager.generated.h"

class IMSaveable;
class UMSaveGame;
class UMSaveIndex;
class UMSaveNode;
struct FKeyEvent;
struct FMSlotId;

DECLARE_LOG_CATEGORY_EXTERN(LogMSaveManager, Log, All);

#pragma region Delegates

/** Delegate called when a save slot is updated (new node, slot changed, etc.). Passes in the UMSaveGame. */
DECLARE_MULTICAST_DELEGATE_OneParam(FMOnSaveSlotUpdatedDelegate, UMSaveGame*);

/**
 * Delegate called from AsyncSaveGame. First two parameters are contextual SlotName and UserIndex,
 * third parameter is the resulting UMSaveNode (null on failure).
 */
MEMENTOSAVESYSTEM_API DECLARE_DELEGATE_ThreeParams(FMAsyncSaveGameDelegate, const FString&, const int32, UMSaveNode*);

/**
 * Bluprint-exposed delegate called from AsyncSaveGameDynamic. First two paramters are contextual
 * SlotName and UserIndex, third parameter is the resulting UMSaveNode (null on failure).
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncSaveGameDelegateDynamic, const FString&, SlotName, const int32, UserIndex, UMSaveNode*, SaveNode);

/**
 * Delegate called from AsyncLoadGame. First two parameters are contextual SlotName and UserIndex,
 * third parameter is the invisible UMSaveNode automatically created (null on failure).
 */
MEMENTOSAVESYSTEM_API DECLARE_DELEGATE_ThreeParams(FMAsyncLoadGameDelegate, const FString&, const int32, UMSaveNode*);

/**
 * Bluprint-exposed delegate called from AsyncLoadGameDynamic. First two paramters are contextual
 * SlotName and UserIndex, third parameter is the invisible UMSaveNode automatically created (null on failure).
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncLoadGameDelegateDynamic, const FString&, SlotName, const int32, UserIndex, UMSaveNode*, SaveNode);

/**
 * Delegate called from AsyncCreateSaveSlot. First two parameters are passed in SlotName and UserIndex,
 * third parameter is the resulting UMSaveGame (null on failure).
 */
MEMENTOSAVESYSTEM_API DECLARE_DELEGATE_ThreeParams(FMAsyncCreateSlotDelegate, const FString&, const int32, UMSaveGame*);

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
MEMENTOSAVESYSTEM_API DECLARE_DELEGATE_ThreeParams(FMAsyncLoadSlotDelegate, const FString&, const int32, UMSaveGame*);

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
MEMENTOSAVESYSTEM_API DECLARE_DELEGATE_ThreeParams(FMAsyncDeleteSlotDelegate, const FString&, const int32, bool);

/**
 * Bluprint-exposed delegate called from AsyncDeleteSaveSlotDynamic. First two paramters are passed in
 * SlotName and UserIndex, third parameter is whether the operation succeeded.
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FMAsyncDeleteSlotDelegateDynamic, const FString&, SlotName, const int32, UserIndex, bool, bSuccess);

#pragma endregion

#pragma region UMSavemanager

/** A game instance subsystem that handles non-linear save and load operations */
UCLASS()
class MEMENTOSAVESYSTEM_API UMSaveManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Called when a save slot is updated (node creation/deletion, save slot change, etc.) */
	FMOnSaveSlotUpdatedDelegate OnSaveSlotUpdated;

	/**
	 * Creates a node in the save graph and returns it.
	 * Providing no BranchParentId creates a node with both parents pointing to the previous node.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMSaveNode* SaveGame(bool bInvisible = false, FGuid BranchParentId = FGuid());

	/**
	 * Asynchronously creates a node in the save graph and calls a delegate.
	 * Providing no BranchParentId creates a node with both parents pointing to the previous node.
	 */
	void AsyncSaveGame(FMAsyncSaveGameDelegate Delegate, bool bInvisible = false, FGuid BranchParentId = FGuid());

	/**
	 * Asynchronously creates a node in the save graph and calls a delegate.
	 * Providing no BranchParentId creates a node with both parents pointing to the previous node.
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Save Game"))
	void AsyncSaveGameDynamic(
		FMAsyncSaveGameDelegateDynamic Delegate, bool bInvisible = false, FGuid BranchParentId = FGuid());

	/**
	 * Loads a node from the save graph. If bLoadRaw is false, creates an invisible node in the save graph
	 * after loading, which is used to combine separate timelines. Returns a pointer to the invisible node.
	 *
	 * That node's SequenceParent points to the last SaveNode, and its BranchParent points to the provided node to load.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMSaveNode* LoadGame(FGuid SaveId, bool bLoadRaw = false);

	/**
	 * Asynchronously loads a node from the save graph and calls a delegate, if bLoadRaw is false,
	 * creates an invisible node in the save graph after loading, which is used to combine separate timelines.
	 *
	 * That node's SequenceParent points to the last SaveNode, and its BranchParent points to the provided node to load.
	 */
	void AsyncLoadGame(FMAsyncLoadGameDelegate Delegate, FGuid SaveId, bool bLoadRaw = false);

	/**
	 * Asynchronously loads a node from the save graph and calls a delegate, if bLoadRaw is false,
	 * creates an invisible node in the save graph after loading, which is used to combine separate timelines.
	 *
	 * That node's SequenceParent points to the last SaveNode, and its BranchParent points to the provided node to load.
	 *
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Load Game"))
	void AsyncLoadGameDynamic(FMAsyncLoadGameDelegateDynamic Delegate, FGuid SaveId, bool bLoadRaw = false);

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
	 * Bluprint-exposed version.
	 */
	void AsyncDeleteSaveSlot(FMAsyncDeleteSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex);

	/**
	 * Asynchronously deletes a save slot and all of its save nodes and calls a delegate.
	 * Bluprint-exposed version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (DisplayName = "Async Delete Save Slot"))
	void AsyncDeleteSaveSlotDynamic(
		FMAsyncDeleteSlotDelegateDynamic Delegate, const FString& SlotName, const int32 UserIndex);

	/** Returns the index of all known save slots */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Save System")
	TArray<FMSlotId> GetSaveIndex() const;

	/** Returns the currently active save slot */
	UMSaveGame* GetActiveSaveGame() const { return ActiveSaveGame; }

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

	/** An internal listener for OnSaveSlotUpdated */
	void OnSaveSlotUpdated_Internal(UMSaveGame* SaveGame);

	/** Finds all saveables in the world */
	void FindSaveables(TArray<UObject*>& OutSaveables) const;

	/** Creates a new save node and adds it to the save graph */
	UMSaveNode* CreateSaveNode(bool bInvisible = false, FGuid BranchParentId = FGuid());

	/** Deserializes a save node and triggers the game to load it */
	bool LoadSaveNode(UMSaveNode* SaveNode);

	/** Deletes the save nodes within a slot. Does not delete the slot itself. */
	void DeleteSaveGraph(UMSaveGame* SaveGame);

	/** Handles debug key inputs for quick save and load */
	void HandleKeyDown(const FKeyEvent& EventArgs);

	FDelegateHandle KeyDownDelegateHandle;

	/** Refreshes the auto complete for the console commands */
	void RefreshConsoleCommands();

	/** Registers auto completion for the console commands */
	void HandleRegisterConsoleAutoCompleteEntries(TArray<FAutoCompleteCommand>& OutCommands);

	FDelegateHandle AutoCompleteDelegateHandle;

	/** Calls SaveGame() */
	void ConsoleSaveGame();

	/** Calls LoadGame(saveId, false) and attempts to load the node with the provided save id */
	void ConsoleLoadGame(const TArray<FString>& Args);

	/** Calls LoadGame(saveId, true) and attempts to load the node with the provided save id */
	void ConsoleLoadGameRaw(const TArray<FString>& Args);

	/** Calls CreateSaveSlot(slotName, userIndex) */
	void ConsoleCreateSaveSlot(const TArray<FString>& Args);

	/** Calls LoadSaveSlot(slotName, userIndex) */
	void ConsoleLoadSaveSlot(const TArray<FString>& Args);

	/** Calls DeleteSaveSlot(slotName, userIndex) */
	void ConsoleDeleteSaveSlot(const TArray<FString>& Args);
};

#pragma endregion
