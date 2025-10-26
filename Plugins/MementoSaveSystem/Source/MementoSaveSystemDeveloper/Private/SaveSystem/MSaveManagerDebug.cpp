// Copyright © Bedrockbreaker 2025. MIT License

#include "SaveSystem/MSaveManagerDebug.h"

#include "ConsoleSettings.h"
#include "CoreGlobals.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "InputCoreTypes.h"
#include "SaveSystem/MSaveGame.h"
#include "SaveSystem/MSaveManager.h"
#include "SaveSystem/MSaveNode.h"
#include "SaveSystem/MSlotId.h"

void UMSaveManagerDebug::Initialize(UMSaveManager* SaveManagerIn)
{
	this->SaveManager = SaveManagerIn;
	SaveManager->OnSaveSlotUpdated.AddUObject(this, &UMSaveManagerDebug::OnSaveSlotUpdated);

	RefreshConsoleCommands();

	KeyDownDelegateHandle = FSlateApplication::Get().OnApplicationPreInputKeyDownListener().AddUObject(
		this, &UMSaveManagerDebug::HandleKeyDown);

	AutoCompleteDelegateHandle = UConsole::RegisterConsoleAutoCompleteEntries.AddUObject(
		this, &UMSaveManagerDebug::HandleRegisterConsoleAutoCompleteEntries);

	IConsoleManager& ConsoleManager = IConsoleManager::Get();

	ConsoleManager.RegisterConsoleCommand(
		TEXT("MSaveManager.Save"),
		TEXT("Save the game into the active save slot"),
		FConsoleCommandDelegate::CreateUObject(this, &UMSaveManagerDebug::ConsoleSaveGame),
		ECVF_Cheat);

	ConsoleManager.RegisterConsoleCommand(
		TEXT("MSaveManager.Load"),
		TEXT("Load a save node by GUID"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManagerDebug::ConsoleLoadGame),
		ECVF_Cheat);

	ConsoleManager.RegisterConsoleCommand(
		TEXT("MSaveManager.Recall"),
		TEXT("Recall to a save node by GUID"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManagerDebug::ConsoleRecallGame),
		ECVF_Cheat);

	ConsoleManager.RegisterConsoleCommand(
		TEXT("MSaveManager.CreateSlot"),
		TEXT("Create a new save slot"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManagerDebug::ConsoleCreateSaveSlot),
		ECVF_Cheat);

	ConsoleManager.RegisterConsoleCommand(
		TEXT("MSaveManager.LoadSlot"),
		TEXT("Load a save slot by name and user index"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManagerDebug::ConsoleLoadSaveSlot),
		ECVF_Cheat);

	ConsoleManager.RegisterConsoleCommand(
		TEXT("MSaveManager.DeleteSlot"),
		TEXT("Delete a save slot by name and user index"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManagerDebug::ConsoleDeleteSaveSlot),
		ECVF_Cheat);

	ConsoleManager.RegisterConsoleCommand(
		TEXT("MSaveManager.CloneSlot"),
		TEXT("Clone a save slot by name and user index to a new name and user index"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManagerDebug::ConsoleCloneSaveSlot),
		ECVF_Cheat);
}

void UMSaveManagerDebug::Deinitialize()
{
	SaveManager->OnSaveSlotUpdated.RemoveAll(this);

	if (FSlateApplication::IsInitialized())
		FSlateApplication::Get().OnApplicationPreInputKeyDownListener().Remove(KeyDownDelegateHandle);

	if (UConsole::RegisterConsoleAutoCompleteEntries.IsBound())
		UConsole::RegisterConsoleAutoCompleteEntries.Remove(AutoCompleteDelegateHandle);

	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	ConsoleManager.UnregisterConsoleObject(TEXT("MSaveManager.Save"));
	ConsoleManager.UnregisterConsoleObject(TEXT("MSaveManager.Load"));
	ConsoleManager.UnregisterConsoleObject(TEXT("MSaveManager.Recall"));
	ConsoleManager.UnregisterConsoleObject(TEXT("MSaveManager.CreateSlot"));
	ConsoleManager.UnregisterConsoleObject(TEXT("MSaveManager.LoadSlot"));
	ConsoleManager.UnregisterConsoleObject(TEXT("MSaveManager.DeleteSlot"));
	ConsoleManager.UnregisterConsoleObject(TEXT("MSaveManager.CloneSlot"));

	SaveManager = nullptr;
}

void UMSaveManagerDebug::OnSaveSlotUpdated(UMSaveGame* SaveGame)
{
	RefreshConsoleCommands();
}

void UMSaveManagerDebug::HandleKeyDown(const FKeyEvent& Event)
{
	UMSaveGame* ActiveSaveGame = SaveManager->GetActiveSaveGame();

	if (Event.GetKey() == EKeys::NumPadOne)
	{
		// Load the active node's SequenceParent
		ConsoleLoadGame({ ActiveSaveGame->SaveNodes[ActiveSaveGame->MostRecentNodeId].SequenceParentId.ToString() });
	}
	else if (Event.GetKey() == EKeys::NumPadTwo)
	{
		// Load the active node
		ConsoleLoadGame({ ActiveSaveGame->MostRecentNodeId.ToString() });
	}
	else if (Event.GetKey() == EKeys::NumPadThree)
	{
		// Load the node whose SequenceParent is the active node
		bool bFound = false;
		for (const TTuple<FGuid, FMSaveNodeMetadata>& SaveNode : ActiveSaveGame->SaveNodes)
		{
			if (SaveNode.Value.SequenceParentId == ActiveSaveGame->MostRecentNodeId)
			{
				ConsoleLoadGame({ SaveNode.Key.ToString() });
				bFound = true;
				break;
			}
		}
		if (!bFound && GEngine)
			GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, TEXT("Failed to load save (no children exist)"));
	}
	else if (Event.GetKey() == EKeys::NumPadFive)
	{
		// Save the game
		ConsoleSaveGame();
	}
}

void UMSaveManagerDebug::RefreshConsoleCommands()
{
	if (!GEngine) return;
	if (!GEngine->GameViewport) return;

	UConsole* Console = GEngine->GameViewport->ViewportConsole;
	if (!Console) return;

	// Mark commands as out of date, rebuild on next input
	Console->bIsRuntimeAutoCompleteUpToDate = false;
}

void UMSaveManagerDebug::HandleRegisterConsoleAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteEntries)
{
	FColor TextColor = GetDefault<UConsoleSettings>()->AutoCompleteCommandColor;

	// Generate autocompletion for save slots
	for (const FMSlotId& SlotId : SaveManager->GetSaveIndex())
	{
		FString FullCmdLoad = FString::Printf(TEXT("MSaveManager.LoadSlot %s %d"), *SlotId.SlotName, SlotId.UserIndex);
		FString FullCmdDelete =
			FString::Printf(TEXT("MSaveManager.DeleteSlot %s %d"), *SlotId.SlotName, SlotId.UserIndex);
		FString FullCmdClone =
			FString::Printf(TEXT("MSaveManager.CloneSlot %s %d"), *SlotId.SlotName, SlotId.UserIndex);

		int32 FoundIndex = INDEX_NONE;
		for (int32 i = 0; i < AutoCompleteEntries.Num(); ++i)
		{
			if (AutoCompleteEntries[i].Command == FullCmdLoad)
			{
				FoundIndex = i;
				break;
			}
		}

		int32 NewIndex = FoundIndex == INDEX_NONE ? AutoCompleteEntries.AddDefaulted(3) : FoundIndex;

		// MSaveManager.LoadSlot <SlotName> <UserIndex>
		AutoCompleteEntries[NewIndex].Command = FullCmdLoad;
		AutoCompleteEntries[NewIndex].Color = TextColor;

		// MSaveManager.DeleteSlot <SlotName> <UserIndex>
		AutoCompleteEntries[NewIndex + 1].Command = FullCmdDelete;
		AutoCompleteEntries[NewIndex + 1].Color = TextColor;

		// MSaveManager.CloneSlot <SlotName> <UserIndex>
		AutoCompleteEntries[NewIndex + 2].Command = FullCmdClone;
		AutoCompleteEntries[NewIndex + 2].Color = TextColor;
	}

	UMSaveGame* ActiveSaveGame = SaveManager->GetActiveSaveGame();
	if (!ActiveSaveGame) return;

	// Generate autocompletion for loading save nodes
	for (const TTuple<FGuid, FMSaveNodeMetadata>& Tuple : ActiveSaveGame->SaveNodes)
	{
		FString SaveId = Tuple.Key.ToString();
		FString FullCmd = FString::Printf(TEXT("MSaveManager.Load %s"), *SaveId);
		FString FullCmdRaw = FString::Printf(TEXT("MSaveManager.Recall %s"), *SaveId);

		int32 FoundIndex = INDEX_NONE;
		for (int32 i = 0; i < AutoCompleteEntries.Num(); ++i)
		{
			if (AutoCompleteEntries[i].Command == FullCmd)
			{
				FoundIndex = i;
				break;
			}
		}

		int32 NewIndex = FoundIndex == INDEX_NONE ? AutoCompleteEntries.AddDefaulted(2) : FoundIndex;

		// MSaveSystem.Load <SaveId>
		AutoCompleteEntries[NewIndex].Command = FullCmd;
		AutoCompleteEntries[NewIndex].Desc = Tuple.Value.Timestamp.ToString();
		AutoCompleteEntries[NewIndex].Color = TextColor;

		// MSaveSystem.LoadRaw <SaveId>
		AutoCompleteEntries[NewIndex + 1].Command = FullCmdRaw;
		AutoCompleteEntries[NewIndex + 1].Desc = Tuple.Value.Timestamp.ToString();
		AutoCompleteEntries[NewIndex + 1].Color = TextColor;
	}
}

void UMSaveManagerDebug::ConsoleSaveGame()
{
	UMSaveNode* SaveNode = SaveManager->SaveGame();

	if (!GEngine) return;

	UMSaveGame* ActiveSaveGame = SaveManager->GetActiveSaveGame();
	if (!ActiveSaveGame) return;

	if (SaveNode)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Green,
			FString::Printf(
				TEXT("Saved game - %s:%d (%s)"),
				*ActiveSaveGame->SlotName,
				ActiveSaveGame->UserIndex,
				*SaveNode->SaveId.ToString()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Red,
			FString::Printf(TEXT("Failed to save game - %s:%d"), *ActiveSaveGame->SlotName, ActiveSaveGame->UserIndex));
	}
}

void UMSaveManagerDebug::ConsoleLoadGame(const TArray<FString>& Args)
{
	if (Args.Num() != 1)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.Load <SaveId>"));
		return;
	}

	FGuid SaveId;
	if (!FGuid::Parse(Args[0], SaveId))
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, FString::Printf(TEXT("Invalid Guid: %s"), *Args[0]));
		return;
	}

	UMSaveNode* SaveNode = SaveManager->LoadGame(SaveId);

	if (!GEngine) return;

	UMSaveGame* ActiveSaveGame = SaveManager->GetActiveSaveGame();
	if (!ActiveSaveGame) return;

	if (SaveNode)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Green,
			FString::Printf(
				TEXT("Loaded save - %s:%d (%s)"),
				*ActiveSaveGame->SlotName,
				ActiveSaveGame->UserIndex,
				*SaveNode->SaveId.ToString()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			0, 5.0f, FColor::Red, FString::Printf(TEXT("Failed to load save - %s (Does it exist?)"), *Args[0]));
	}
}

void UMSaveManagerDebug::ConsoleRecallGame(const TArray<FString>& Args)
{
	if (Args.Num() < 1 || Args.Num() > 2)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.Recall <BranchSaveId> [SequenceSaveId]"));
		return;
	}

	FGuid BranchId;
	if (!FGuid::Parse(Args[0], BranchId))
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, FString::Printf(TEXT("Invalid Guid: %s"), *Args[0]));
		return;
	}

	UMSaveGame* ActiveSaveGame = SaveManager->GetActiveSaveGame();
	if (!ActiveSaveGame) return;

	FGuid SequenceId;
	if (Args.Num() <= 1 || !FGuid::Parse(Args[1], SequenceId))
		SequenceId = SaveManager->GetActiveSaveGame()->MostRecentNodeId;

	UMSaveNode* SaveNode = SaveManager->RecallGame(BranchId, SequenceId);

	if (!GEngine) return;

	if (SaveNode)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Green,
			FString::Printf(
				TEXT("Recalled save - %s:%d (%s)"),
				*ActiveSaveGame->SlotName,
				ActiveSaveGame->UserIndex,
				*BranchId.ToString()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			0, 5.0f, FColor::Red, FString::Printf(TEXT("Failed to recall save (Does %s exist?)"), *Args[0]));
	}
}

void UMSaveManagerDebug::ConsoleCreateSaveSlot(const TArray<FString>& Args)
{
	if (Args.Num() != 2)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.SaveSlot <SlotName> <UserIndex>"));
		return;
	}

	UMSaveGame* SaveGame = SaveManager->CreateSaveSlot(Args[0], FCString::Atoi(*Args[1]));

	if (!GEngine) return;

	if (SaveGame)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Green,
			FString::Printf(TEXT("Created save slot - %s:%d"), *Args[0], FCString::Atoi(*Args[1])));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Red,
			FString::Printf(TEXT("Failed to create save slot - %s:%d"), *Args[0], FCString::Atoi(*Args[1])));
	}
}

void UMSaveManagerDebug::ConsoleLoadSaveSlot(const TArray<FString>& Args)
{
	if (Args.Num() != 2)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.LoadSlot <SlotName> <UserIndex>"));
		return;
	}

	UMSaveGame* SaveGame = SaveManager->LoadSaveSlot(Args[0], FCString::Atoi(*Args[1]));

	if (!GEngine) return;

	if (SaveGame)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Green,
			FString::Printf(TEXT("Loaded save slot - %s:%d"), *Args[0], FCString::Atoi(*Args[1])));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Red,
			FString::Printf(TEXT("Failed to load save slot - %s:%d"), *Args[0], FCString::Atoi(*Args[1])));
	}
}

void UMSaveManagerDebug::ConsoleDeleteSaveSlot(const TArray<FString>& Args)
{
	if (Args.Num() != 2)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.DeleteSlot <SlotName> <UserIndex>"));
		return;
	}

	bool bSuccess = SaveManager->DeleteSaveSlot(Args[0], FCString::Atoi(*Args[1]));

	if (!GEngine) return;

	if (bSuccess)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Green,
			FString::Printf(TEXT("Deleted save slot - %s:%d"), *Args[0], FCString::Atoi(*Args[1])));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Red,
			FString::Printf(TEXT("Failed to delete save slot - %s:%d"), *Args[0], FCString::Atoi(*Args[1])));
	}
}

void UMSaveManagerDebug::ConsoleCloneSaveSlot(const TArray<FString>& Args)
{
	if (Args.Num() != 4)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				0,
				5.0f,
				FColor::Red,
				TEXT(
					"Usage: MSaveSystem.CloneSlot <OriginalSlotName> <OriginalUserIndex> <NewSlotName> <NewUserIndex>"));
		return;
	}

	UMSaveGame* SaveGame =
		SaveManager->CloneSaveSlot(Args[0], FCString::Atoi(*Args[1]), Args[2], FCString::Atoi(*Args[3]));

	if (!GEngine) return;

	if (SaveGame)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Green,
			FString::Printf(
				TEXT("Cloned save slot - %s:%d -> %s:%d"),
				*Args[0],
				FCString::Atoi(*Args[1]),
				*Args[2],
				FCString::Atoi(*Args[3])));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Red,
			FString::Printf(
				TEXT("Failed to clone save slot - %s:%d -> %s:%d"),
				*Args[0],
				FCString::Atoi(*Args[1]),
				*Args[2],
				FCString::Atoi(*Args[3])));
	}
}
