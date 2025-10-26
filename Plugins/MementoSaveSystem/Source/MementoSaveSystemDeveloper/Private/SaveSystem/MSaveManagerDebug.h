// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "MSaveManagerDebug.generated.h"

class UMSaveGame;
class UMSaveManager;
struct FKeyEvent;
struct FAutoCompleteCommand;

/** Debug functionality for the UMSaveManager */
UCLASS()
class UMSaveManagerDebug : public UObject
{
	GENERATED_BODY()

public:
	/** Registers console commands and listens for debug key inputs */
	void Initialize(UMSaveManager* SaveManager);

	/** Unregisters console commands and stops listening for debug key inputs */
	void Deinitialize();

private:
	/** Strong reference to the save manager (if the SaveManager isn't valid, we've got bigger issues) */
	UPROPERTY()
	UMSaveManager* SaveManager;

	/** Listener for SaveManager::OnSaveSlotUpdated */
	void OnSaveSlotUpdated(UMSaveGame* SaveGame);

	/** Handles debug key inputs for quick save and load */
	void HandleKeyDown(const FKeyEvent& EventArgs);

	/** Listens for debug key inputs for quick save and load */
	FDelegateHandle KeyDownDelegateHandle;

	/** Refreshes the auto complete for the console commands */
	void RefreshConsoleCommands();

	/** Handles auto completion for the console commands */
	void HandleRegisterConsoleAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteEntries);

	/** Listens for auto completion requests for the console commands */
	FDelegateHandle AutoCompleteDelegateHandle;

	/** Calls MSaveManager::SaveGame() */
	void ConsoleSaveGame();

	/** Calls MSaveManager::LoadGame(saveId) */
	void ConsoleLoadGame(const TArray<FString>& Args);

	/** Calls MSaveManager::RecallGame(branchId, sequenceId) */
	void ConsoleRecallGame(const TArray<FString>& Args);

	/** Calls MSaveManager::CreateSaveSlot(slotName, userIndex) */
	void ConsoleCreateSaveSlot(const TArray<FString>& Args);

	/** Calls MSaveManager::LoadSaveSlot(slotName, userIndex) */
	void ConsoleLoadSaveSlot(const TArray<FString>& Args);

	/** Calls MSaveManager::DeleteSaveSlot(slotName, userIndex) */
	void ConsoleDeleteSaveSlot(const TArray<FString>& Args);

	/** Calls MSaveManager::CloneSaveSlot(originalSlotName, originalUserIndex, newSlotName, newUserIndex) */
	void ConsoleCloneSaveSlot(const TArray<FString>& Args);
};
