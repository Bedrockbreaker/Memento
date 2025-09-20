// Copyright © Bedrockbreaker 2025. MIT License

#include "SaveSystem/MSaveManager.h"

#include "ConsoleSettings.h"
#include "Containers/UnrealString.h"
#include "CoreGlobals.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/IMSaveable.h"
#include "SaveSystem/MSaveData.h"
#include "SaveSystem/MSaveGame.h"
#include "SaveSystem/MSaveIndex.h"
#include "SaveSystem/MSaveNode.h"
#include "SaveSystem/MSaveNodeMetadata.h"
#include "SaveSystem/MSlotId.h"
#include "Serialization/Archive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Subsystems/SubsystemCollection.h"
#include "UObject/ScriptInterface.h"

DEFINE_LOG_CATEGORY(LogMSaveManager);

UMSaveNode* UMSaveManager::SaveGame(bool bInvisible, FGuid BranchParentId)
{
	checkf(ActiveSaveGame, TEXT("A save slot must be created/loaded before saving."));

	UMSaveNode* SaveNode = CreateSaveNode(bInvisible, BranchParentId);
	if (!SaveNode) return nullptr;

	UE_LOG(
		LogMSaveManager,
		Log,
		TEXT("Saving game - %s:%d (%s)"),
		*ActiveSaveGame->SlotName,
		ActiveSaveGame->UserIndex,
		*SaveNode->SaveId.ToString());

	FString SlotName = ActiveSaveGame->SlotName + SaveNode->SaveId.ToString();

	bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveNode, SlotName, ActiveSaveGame->UserIndex);
	bSuccess = bSuccess
		&& UGameplayStatics::SaveGameToSlot(ActiveSaveGame, ActiveSaveGame->SlotName, ActiveSaveGame->UserIndex);

	if (bSuccess) OnSaveSlotUpdated.Broadcast(ActiveSaveGame);

	return bSuccess ? SaveNode : nullptr;
}

void UMSaveManager::AsyncSaveGame(FMAsyncSaveGameDelegate Delegate, bool bInvisible, FGuid BranchParentId)
{
	checkf(ActiveSaveGame, TEXT("A save slot must be created/loaded before saving."));

	UMSaveNode* SaveNode = CreateSaveNode(bInvisible, BranchParentId);
	if (!SaveNode)
	{
		Delegate.ExecuteIfBound(ActiveSaveGame->SlotName, ActiveSaveGame->UserIndex, nullptr);
		return;
	}

	FAsyncSaveGameToSlotDelegate SaveDelegate;
	if (Delegate.IsBound())
	{
		SaveDelegate.BindLambda(
			[Delegate, SaveNode, this](const FString& SlotName, const int32 UserIndex, bool bSuccess) -> void {
				if (bSuccess) OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
				Delegate.ExecuteIfBound(SlotName, UserIndex, bSuccess ? SaveNode : nullptr);
			});
	}

	UE_LOG(
		LogMSaveManager,
		Log,
		TEXT("Saving game - %s:%d (%s)"),
		*ActiveSaveGame->SlotName,
		ActiveSaveGame->UserIndex,
		*SaveNode->SaveId.ToString());

	FString SlotName = ActiveSaveGame->SlotName + SaveNode->SaveId.ToString();

	UGameplayStatics::AsyncSaveGameToSlot(SaveNode, SlotName, ActiveSaveGame->UserIndex, MoveTemp(SaveDelegate));
	// TODO: the slot save also needs to be factored in for returning success
	UGameplayStatics::AsyncSaveGameToSlot(ActiveSaveGame, ActiveSaveGame->SlotName, ActiveSaveGame->UserIndex, nullptr);
}

void UMSaveManager::AsyncSaveGameDynamic(FMAsyncSaveGameDelegateDynamic Delegate, bool bInvisible, FGuid BranchParentId)
{
	FMAsyncSaveGameDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda(
			[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveNode* SaveNode) -> void {
				Delegate.ExecuteIfBound(SlotName, UserIndex, SaveNode);
			});
	}

	AsyncSaveGame(MoveTemp(NativeDelegate), bInvisible, BranchParentId);
}

UMSaveNode* UMSaveManager::LoadGame(FGuid SaveId, bool bLoadRaw)
{
	checkf(ActiveSaveGame, TEXT("A save slot must be created/loaded before loading."));

	UE_LOG(
		LogMSaveManager,
		Log,
		TEXT("Loading game - %s:%d (%s)"),
		*ActiveSaveGame->SlotName,
		ActiveSaveGame->UserIndex,
		*SaveId.ToString());

	FString		SlotName = ActiveSaveGame->SlotName + SaveId.ToString();
	UMSaveNode* SaveNode = Cast<UMSaveNode>(UGameplayStatics::LoadGameFromSlot(SlotName, ActiveSaveGame->UserIndex));

	bool bSuccess = LoadSaveNode(SaveNode);
	if (bSuccess)
	{
		if (bLoadRaw) ActiveSaveGame->MostRecentNodeId = SaveId;
		OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
	}

	if (bLoadRaw || !bSuccess) return nullptr;

	// Create an invisible node to combine separate timelines
	UMSaveNode* InvisibleNode = SaveGame(true, SaveId);
	return InvisibleNode;
}

void UMSaveManager::AsyncLoadGame(FMAsyncLoadGameDelegate Delegate, FGuid SaveId, bool bLoadRaw)
{
	checkf(ActiveSaveGame, TEXT("A save slot must be created/loaded before loading."));

	UE_LOG(
		LogMSaveManager,
		Log,
		TEXT("Loading game - %s:%d (%s)"),
		*ActiveSaveGame->SlotName,
		ActiveSaveGame->UserIndex,
		*SaveId.ToString());

	FString SlotName = ActiveSaveGame->SlotName + SaveId.ToString();

	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindLambda(
		[Delegate, SaveId, bLoadRaw, this](
			const FString& InSlotName, const int32 InUserIndex, USaveGame* SaveNode) -> void {
			bool bSuccess = SaveNode != nullptr;

			if (bSuccess)
			{
				if (bLoadRaw) ActiveSaveGame->MostRecentNodeId = SaveId;
				OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
			}

			if (bLoadRaw || !bSuccess)
			{
				Delegate.ExecuteIfBound(InSlotName, InUserIndex, nullptr);
				return;
			}

			// Create an invisible node to combine separate timelines
			FMAsyncSaveGameDelegate SaveDelegate;
			SaveDelegate.BindLambda(
				[Delegate](const FString& InSlotName, const int32 InUserIndex, UMSaveNode* SaveNode) -> void {
					Delegate.ExecuteIfBound(InSlotName, InUserIndex, SaveNode);
				});

			AsyncSaveGame(MoveTemp(SaveDelegate), true, SaveId);
		});

	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, ActiveSaveGame->UserIndex, MoveTemp(LoadDelegate));
}

void UMSaveManager::AsyncLoadGameDynamic(FMAsyncLoadGameDelegateDynamic Delegate, FGuid SaveId, bool bLoadRaw)
{
	FMAsyncLoadGameDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda(
			[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveNode* SaveNode) -> void {
				Delegate.ExecuteIfBound(SlotName, UserIndex, SaveNode);
			});
	}

	AsyncLoadGame(MoveTemp(NativeDelegate), SaveId, bLoadRaw);
}

UMSaveGame* UMSaveManager::CreateSaveSlot(const FString& SlotName, const int32 UserIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex)) DeleteSaveSlot(SlotName, UserIndex);

	UE_LOG(LogMSaveManager, Log, TEXT("Creating save slot - %s:%d"), *SlotName, UserIndex);

	UMSaveGame* SaveGame = Cast<UMSaveGame>(UGameplayStatics::CreateSaveGameObject(UMSaveGame::StaticClass()));
	SaveGame->SlotName = SlotName;
	SaveGame->UserIndex = UserIndex;

	bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

	if (bSuccess && SaveIndex)
	{
		SaveIndex->SaveSlots.Add({ SlotName, UserIndex });
		bSuccess = bSuccess && UGameplayStatics::SaveGameToSlot(SaveIndex, TEXT("SaveIndex"), 0);
		// TODO: Index updated event
	}

	if (bSuccess)
	{
		ActiveSaveGame = SaveGame;
		OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
	}
	else
	{
		UE_LOG(LogMSaveManager, Warning, TEXT("Failed to create save slot"));
	}

	return bSuccess ? SaveGame : nullptr;
}

void UMSaveManager::AsyncCreateSaveSlot(
	FMAsyncCreateSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex)
{
	FMAsyncDeleteSlotDelegate NativeDelegate = FMAsyncDeleteSlotDelegate::CreateLambda(
		[Delegate, this](const FString& InSlotName, const int32 InUserIndex, bool bSuccess) -> void {
			UMSaveGame* SaveGame = Cast<UMSaveGame>(UGameplayStatics::CreateSaveGameObject(UMSaveGame::StaticClass()));
			SaveGame->SlotName = InSlotName;
			SaveGame->UserIndex = InUserIndex;

			FAsyncSaveGameToSlotDelegate SaveDelegate = FAsyncSaveGameToSlotDelegate::CreateLambda(
				[Delegate, SaveGame, this](const FString& SlotName, const int32 UserIndex, bool bSuccess) -> void {
					FAsyncSaveGameToSlotDelegate SaveIndexDelegate = FAsyncSaveGameToSlotDelegate::CreateLambda(
						[Delegate, SaveGame, this](
							const FString& SlotName, const int32 UserIndex, bool bSuccess) -> void {
							// TODO: Index updated event
							if (bSuccess)
							{
								ActiveSaveGame = SaveGame;
								OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
							}

							Delegate.ExecuteIfBound(SlotName, UserIndex, bSuccess ? SaveGame : nullptr);
						});

					if (bSuccess && SaveIndex)
					{
						SaveIndex->SaveSlots.Add({ SlotName, UserIndex });
						UGameplayStatics::AsyncSaveGameToSlot(
							SaveIndex, TEXT("SaveIndex"), 0, MoveTemp(SaveIndexDelegate));
					}
					else
					{
						SaveIndexDelegate.Execute(SlotName, UserIndex, bSuccess);
					}
				});

			UGameplayStatics::AsyncSaveGameToSlot(SaveGame, InSlotName, InUserIndex, MoveTemp(SaveDelegate));
		});

	UE_LOG(LogMSaveManager, Log, TEXT("Creating save slot - %s:%d"), *SlotName, UserIndex);

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		AsyncDeleteSaveSlot(MoveTemp(NativeDelegate), SlotName, UserIndex);
	}
	else
	{
		NativeDelegate.Execute(SlotName, UserIndex, true);
	}
}

void UMSaveManager::AsyncCreateSaveSlotDynamic(
	FMAsyncCreateSlotDelegateDynamic Delegate, const FString& SlotName, const int32 UserIndex)
{
	FMAsyncCreateSlotDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda(
			[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveGame* SaveGame) -> void {
				Delegate.ExecuteIfBound(SlotName, UserIndex, SaveGame);
			});
	}

	AsyncCreateSaveSlot(MoveTemp(NativeDelegate), SlotName, UserIndex);
}

UMSaveGame* UMSaveManager::LoadSaveSlot(const FString& SlotName, const int32 UserIndex, bool bSetActive)
{
	UE_LOG(LogMSaveManager, Log, TEXT("Loading save slot - %s:%d"), *SlotName, UserIndex);

	UMSaveGame* SaveGame = Cast<UMSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if (bSetActive && SaveGame)
	{
		ActiveSaveGame = SaveGame;
		OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
	}
	return SaveGame;
}

void UMSaveManager::AsyncLoadSaveSlot(
	FMAsyncLoadSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex, bool bSetActive)
{
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindLambda(
		[Delegate, bSetActive, this](const FString& SlotName, const int32 UserIndex, USaveGame* SaveGame) -> void {
			UMSaveGame* MSaveGame = Cast<UMSaveGame>(SaveGame);
			if (bSetActive && MSaveGame)
			{
				ActiveSaveGame = MSaveGame;
				OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
			}

			Delegate.ExecuteIfBound(SlotName, UserIndex, MSaveGame);
		});

	UE_LOG(LogMSaveManager, Log, TEXT("Loading save slot - %s:%d"), *SlotName, UserIndex);

	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, UserIndex, MoveTemp(LoadDelegate));
}

void UMSaveManager::AsyncLoadSaveSlotDynamic(
	FMAsyncLoadSlotDelegateDynamic Delegate, const FString& SlotName, const int32 UserIndex, bool bSetActive)
{
	FMAsyncLoadSlotDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda(
			[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveGame* SaveGame) -> void {
				Delegate.ExecuteIfBound(SlotName, UserIndex, SaveGame);
			});
	}

	AsyncLoadSaveSlot(MoveTemp(NativeDelegate), SlotName, UserIndex, bSetActive);
}

UMSaveGame* UMSaveManager::LoadOrCreateSaveSlot(const FString& SlotName, const int32 UserIndex)
{
	UMSaveGame* SaveGame = LoadSaveSlot(SlotName, UserIndex);
	if (!SaveGame) SaveGame = CreateSaveSlot(SlotName, UserIndex);
	return SaveGame;
}

void UMSaveManager::AsyncLoadOrCreateSaveSlot(
	FMAsyncLoadSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex)
{
	bool bSlotExists = UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
	if (bSlotExists)
	{
		AsyncLoadSaveSlot(MoveTemp(Delegate), SlotName, UserIndex, true);
	}
	else
	{
		FMAsyncCreateSlotDelegate CreateDelegate;
		if (Delegate.IsBound())
		{
			CreateDelegate.BindLambda(
				[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveGame* SaveGame) -> void {
					Delegate.ExecuteIfBound(SlotName, UserIndex, SaveGame);
				});
		}

		AsyncCreateSaveSlot(MoveTemp(CreateDelegate), SlotName, UserIndex);
	}
}

void UMSaveManager::AsyncLoadOrCreateSaveSlotDynamic(
	FMAsyncLoadSlotDelegateDynamic Delegate, const FString& SlotName, const int32 UserIndex)
{
	FMAsyncLoadSlotDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda(
			[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveGame* SaveGame) -> void {
				Delegate.ExecuteIfBound(SlotName, UserIndex, SaveGame);
			});
	}

	AsyncLoadOrCreateSaveSlot(MoveTemp(NativeDelegate), SlotName, UserIndex);
}

bool UMSaveManager::DeleteSaveSlot(const FString& SlotName, const int32 UserIndex)
{
	UMSaveGame* SaveGame = LoadSaveSlot(SlotName, UserIndex, false);
	if (!SaveGame) return false;

	UE_LOG(LogMSaveManager, Log, TEXT("Deleting save slot - %s:%d"), *SlotName, UserIndex);

	DeleteSaveGraph(SaveGame);

	if (ActiveSaveGame && SaveGame->SlotName == ActiveSaveGame->SlotName
		&& SaveGame->UserIndex == ActiveSaveGame->UserIndex)
	{
		ActiveSaveGame = nullptr;
		OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
	}

	SaveIndex->SaveSlots.Remove({ SlotName, UserIndex });
	UGameplayStatics::SaveGameToSlot(SaveIndex, TEXT("SaveIndex"), 0);

	// TODO: Index updated event

	return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

void UMSaveManager::AsyncDeleteSaveSlot(
	FMAsyncDeleteSlotDelegate Delegate, const FString& SlotName, const int32 UserIndex)
{
	FMAsyncLoadSlotDelegate LoadDelegate;
	LoadDelegate.BindLambda(
		[Delegate, this](const FString& InSlotName, const int32 InUserIndex, UMSaveGame* SaveGame) -> void {
			if (!SaveGame)
			{
				Delegate.ExecuteIfBound(InSlotName, InUserIndex, false);
				return;
			}

			UE_LOG(LogMSaveManager, Log, TEXT("Deleting save slot - %s:%d"), *InSlotName, InUserIndex);

			DeleteSaveGraph(SaveGame);

			if (ActiveSaveGame && SaveGame->SlotName == ActiveSaveGame->SlotName
				&& SaveGame->UserIndex == ActiveSaveGame->UserIndex)
			{
				ActiveSaveGame = nullptr;
				OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
			}

			SaveIndex->SaveSlots.Remove({ InSlotName, InUserIndex });
			FAsyncSaveGameToSlotDelegate SaveIndexDelegate = FAsyncSaveGameToSlotDelegate::CreateLambda(
				[Delegate, InSlotName, InUserIndex, this](
					const FString& SlotName, const int32 UserIndex, bool bSuccess) -> void {
					// TODO: Index updated event
					bSuccess = bSuccess && UGameplayStatics::DeleteGameInSlot(InSlotName, InUserIndex);
					Delegate.ExecuteIfBound(InSlotName, InUserIndex, bSuccess);
				});

			UGameplayStatics::AsyncSaveGameToSlot(SaveIndex, TEXT("SaveIndex"), 0, MoveTemp(SaveIndexDelegate));
		});

	AsyncLoadSaveSlot(MoveTemp(LoadDelegate), SlotName, UserIndex, false);
}

void UMSaveManager::AsyncDeleteSaveSlotDynamic(
	FMAsyncDeleteSlotDelegateDynamic Delegate, const FString& SlotName, const int32 UserIndex)
{
	FMAsyncDeleteSlotDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda([Delegate](const FString& SlotName, const int32 UserIndex, bool bSuccess) -> void {
			Delegate.ExecuteIfBound(SlotName, UserIndex, bSuccess);
		});
	}

	AsyncDeleteSaveSlot(MoveTemp(NativeDelegate), SlotName, UserIndex);
}

TArray<FMSlotId> UMSaveManager::GetSaveIndex() const
{
	return SaveIndex ? SaveIndex->SaveSlots : TArray<FMSlotId>();
}

void UMSaveManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!UGameplayStatics::DoesSaveGameExist(TEXT("SaveIndex"), 0))
	{
		UMSaveIndex* Index = Cast<UMSaveIndex>(UGameplayStatics::CreateSaveGameObject(UMSaveIndex::StaticClass()));
		FAsyncSaveGameToSlotDelegate Delegate = FAsyncSaveGameToSlotDelegate::CreateLambda(
			[Index, this](const FString& SlotName, int32 UserIndex, bool bSuccess) -> void {
				if (bSuccess)
					SaveIndex = Index;
				else
					UE_LOG(LogMSaveManager, Warning, TEXT("Failed to create SaveIndex"));
			});
		UGameplayStatics::AsyncSaveGameToSlot(Index, TEXT("SaveIndex"), 0, MoveTemp(Delegate));
	}
	else
	{
		FAsyncLoadGameFromSlotDelegate Delegate = FAsyncLoadGameFromSlotDelegate::CreateLambda(
			[this](const FString& SlotName, int32 UserIndex, USaveGame* SaveGame) -> void {
				UMSaveIndex* Index = Cast<UMSaveIndex>(SaveGame);
				if (Index)
				{
					SaveIndex = Index;
					RefreshConsoleCommands();
				}
				else
				{
					UE_LOG(LogMSaveManager, Warning, TEXT("Failed to load SaveIndex"));
				}
			});

		UGameplayStatics::AsyncLoadGameFromSlot(TEXT("SaveIndex"), 0, MoveTemp(Delegate));
	}

	// #if UE_WITH_CHEAT_MANAGER

	KeyDownDelegateHandle =
		FSlateApplication::Get().OnApplicationPreInputKeyDownListener().AddUObject(this, &UMSaveManager::HandleKeyDown);

	// Register our own listener for OnSaveSlotUpdated
	OnSaveSlotUpdated.AddUObject(this, &UMSaveManager::OnSaveSlotUpdated_Internal);

	// Register to the UConsole to create auto completion entries
	AutoCompleteDelegateHandle = UConsole::RegisterConsoleAutoCompleteEntries.AddUObject(
		this, &UMSaveManager::HandleRegisterConsoleAutoCompleteEntries);

	IConsoleManager& Manager = IConsoleManager::Get();

	Manager.RegisterConsoleCommand(
		TEXT("MSaveManager.Save"),
		TEXT("Save the game into the active save slot"),
		FConsoleCommandDelegate::CreateUObject(this, &UMSaveManager::ConsoleSaveGame),
		ECVF_Cheat);

	Manager.RegisterConsoleCommand(
		TEXT("MSaveManager.Load"),
		TEXT("Load a save node by GUID"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManager::ConsoleLoadGame),
		ECVF_Cheat);

	Manager.RegisterConsoleCommand(
		TEXT("MSaveManager.LoadRaw"),
		TEXT("Load a save node by GUID without creating a new invisible node"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManager::ConsoleLoadGameRaw),
		ECVF_Cheat);

	Manager.RegisterConsoleCommand(
		TEXT("MSaveManager.CreateSlot"),
		TEXT("Create a new save slot"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManager::ConsoleCreateSaveSlot),
		ECVF_Cheat);

	Manager.RegisterConsoleCommand(
		TEXT("MSaveManager.LoadSlot"),
		TEXT("Load a save slot by name and user index"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManager::ConsoleLoadSaveSlot),
		ECVF_Cheat);

	Manager.RegisterConsoleCommand(
		TEXT("MSaveManager.DeleteSlot"),
		TEXT("Delete a save slot by name and user index"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMSaveManager::ConsoleDeleteSaveSlot),
		ECVF_Cheat);

	// #endif
}

void UMSaveManager::Deinitialize()
{
	OnSaveSlotUpdated.RemoveAll(this);

	// #if UE_WITH_CHEAT_MANAGER
	if (FSlateApplication::IsInitialized())
		FSlateApplication::Get().OnApplicationPreInputKeyDownListener().Remove(KeyDownDelegateHandle);

	if (UConsole::RegisterConsoleAutoCompleteEntries.IsBound())
		UConsole::RegisterConsoleAutoCompleteEntries.Remove(AutoCompleteDelegateHandle);

	IConsoleManager& Manager = IConsoleManager::Get();
	Manager.UnregisterConsoleObject(TEXT("MSaveManager.Save"));
	Manager.UnregisterConsoleObject(TEXT("MSaveManager.Load"));
	Manager.UnregisterConsoleObject(TEXT("MSaveManager.LoadRaw"));
	Manager.UnregisterConsoleObject(TEXT("MSaveManager.CreateSlot"));
	Manager.UnregisterConsoleObject(TEXT("MSaveManager.LoadSlot"));
	Manager.UnregisterConsoleObject(TEXT("MSaveManager.DeleteSlot"));
	// #endif

	Super::Deinitialize();
}

void UMSaveManager::OnSaveSlotUpdated_Internal(UMSaveGame* SaveGame)
{
	// #if UE_WITH_CHEAT_MANAGER
	if (IsInGameThread())
	{
		RefreshConsoleCommands();
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this]() -> void { RefreshConsoleCommands(); });
	}
	// #endif
}

void UMSaveManager::FindSaveables(TArray<TScriptInterface<IMSaveable>>& OutSaveables) const
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (Actor->GetClass()->ImplementsInterface(UMSaveable::StaticClass()))
		{
			OutSaveables.Add(Actor);
		}

		// Components may also be saveable
		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (Component->GetClass()->ImplementsInterface(UMSaveable::StaticClass()))
			{
				OutSaveables.Add(Component);
			}
		}
	}
}

UMSaveNode* UMSaveManager::CreateSaveNode(bool bInvisible, FGuid BranchParentId)
{
	if (!ActiveSaveGame) return nullptr;

	FGuid SaveId = FGuid::NewGuid();

	UMSaveNode* SaveNode = Cast<UMSaveNode>(UGameplayStatics::CreateSaveGameObject(UMSaveNode::StaticClass()));
	SaveNode->SaveId = SaveId;

	FMSaveNodeMetadata Metadata;
	Metadata.SaveId = SaveId;
	Metadata.BranchParentId = BranchParentId.IsValid() ? BranchParentId : ActiveSaveGame->MostRecentNodeId;
	Metadata.SequenceParentId = ActiveSaveGame->MostRecentNodeId;
	Metadata.Timestamp = FDateTime::UtcNow();
	Metadata.bInvisible = bInvisible;

	ActiveSaveGame->SaveNodes.Add(SaveId, Metadata);
	ActiveSaveGame->MostRecentNodeId = SaveId;

	UE_LOG(
		LogMSaveManager,
		Log,
		TEXT("Created save node - %s:%d (%s)"),
		*ActiveSaveGame->SlotName,
		ActiveSaveGame->UserIndex,
		*SaveId.ToString());

	TArray<TScriptInterface<IMSaveable>> Saveables;
	FindSaveables(Saveables);

	for (const TScriptInterface<IMSaveable>& Saveable : Saveables)
	{
		if (!Saveable) continue;

		FMSaveData SaveData;
		SaveData.ClassName = Saveable.GetObject()->GetClass()->GetPathName();

		FMemoryWriter Writer(SaveData.Data, true);
		if (Saveable->RequiresCustomSerialization()) Saveable->Save(Writer);
		FObjectAndNameAsStringProxyArchive Archive(Writer, true);
		Archive.ArIsSaveGame = true; // Serialize all properties marked as SaveGame
		Archive.ArNoDelta = true;	 // Blueprint properties don't serialize consistently without this
		Saveable.GetObject()->Serialize(Archive);

		SaveNode->SaveData.Add(Saveable->GetSaveId(), MoveTemp(SaveData));
	}

	return SaveNode;
}

bool UMSaveManager::LoadSaveNode(UMSaveNode* SaveNode)
{
	if (!SaveNode) return false;

	TArray<TScriptInterface<IMSaveable>> Saveables;
	FindSaveables(Saveables);

	for (const TScriptInterface<IMSaveable>& Saveable : Saveables)
	{
		if (!Saveable) continue;

		FMSaveData* SaveData = SaveNode->SaveData.Find(Saveable->GetSaveId());
		// TODO: create runtime-generated objects if they don't exist
		if (!SaveData) continue;

		FMemoryReader Reader(SaveData->Data, true);
		if (Saveable->RequiresCustomSerialization()) Saveable->Load(Reader);
		FObjectAndNameAsStringProxyArchive Archive(Reader, true);
		Archive.ArIsSaveGame = true; // Serialize all properties marked as SaveGame
		Archive.ArNoDelta = true;	 // Blueprint properties don't serialize consistently without this
		Saveable.GetObject()->Serialize(Archive);
	}

	return true;
}

void UMSaveManager::DeleteSaveGraph(UMSaveGame* SaveGame)
{
	if (!SaveGame) return;

	for (const TTuple<FGuid, FMSaveNodeMetadata>& SaveNode : SaveGame->SaveNodes)
	{
		FString SlotName = SaveGame->SlotName + SaveNode.Key.ToString();
		bool	bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, SaveGame->UserIndex);
		if (bSuccess)
		{
			UE_LOG(
				LogMSaveManager,
				Log,
				TEXT("  Deleting save node - %s:%d (%s)"),
				*SaveGame->SlotName,
				SaveGame->UserIndex,
				*SaveNode.Key.ToString());
		}
		else
		{
			UE_LOG(
				LogMSaveManager,
				Warning,
				TEXT("  Failed to delete save node - %s:%d (%s)"),
				*SaveGame->SlotName,
				SaveGame->UserIndex,
				*SaveNode.Key.ToString());
		}
	}
}

void UMSaveManager::HandleKeyDown(const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::NumPadOne)
	{
		// Load the active node's SequenceParent
		ConsoleLoadGameRaw({ ActiveSaveGame->SaveNodes[ActiveSaveGame->MostRecentNodeId].SequenceParentId.ToString() });
	}
	else if (Event.GetKey() == EKeys::NumPadTwo)
	{
		// Load the active node
		ConsoleLoadGameRaw({ ActiveSaveGame->MostRecentNodeId.ToString() });
	}
	else if (Event.GetKey() == EKeys::NumPadThree)
	{
		// Load the node whose SequenceParent is the active node
		bool bFound = false;
		for (const TTuple<FGuid, FMSaveNodeMetadata>& SaveNode : ActiveSaveGame->SaveNodes)
		{
			if (SaveNode.Value.SequenceParentId == ActiveSaveGame->MostRecentNodeId)
			{
				ConsoleLoadGameRaw({ SaveNode.Key.ToString() });
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

void UMSaveManager::RefreshConsoleCommands()
{
	// #if UE_WITH_CHEAT_MANAGER
	if (!GEngine) return;
	if (!GEngine->GameViewport) return;

	UConsole* Console = GEngine->GameViewport->ViewportConsole;
	if (!Console) return;

	// Mark commands as out of date, rebuild on next input
	Console->bIsRuntimeAutoCompleteUpToDate = false;
	// #endif
}

void UMSaveManager::HandleRegisterConsoleAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteEntries)
{
	// #if UE_WITH_CHEAT_MANAGER
	FColor TextColor = GetDefault<UConsoleSettings>()->AutoCompleteCommandColor;

	// Generate autocompletion for save slots
	for (const FMSlotId& SlotId : GetSaveIndex())
	{
		FString FullCmd = FString::Printf(TEXT("MSaveManager.LoadSlot %s %d"), *SlotId.SlotName, SlotId.UserIndex);
		FString FullCmdDelete =
			FString::Printf(TEXT("MSaveManager.DeleteSlot %s %d"), *SlotId.SlotName, SlotId.UserIndex);

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

		// MSaveManager.LoadSlot <SlotName> <UserIndex>
		AutoCompleteEntries[NewIndex].Command = FullCmd;
		AutoCompleteEntries[NewIndex].Color = TextColor;

		// MSaveManager.DeleteSlot <SlotName> <UserIndex>
		AutoCompleteEntries[NewIndex + 1].Command = FullCmdDelete;
		AutoCompleteEntries[NewIndex + 1].Color = TextColor;
	}

	if (!ActiveSaveGame) return;

	// Generate autocompletion for loading save nodes
	for (const TTuple<FGuid, FMSaveNodeMetadata>& Tuple : ActiveSaveGame->SaveNodes)
	{
		FString SaveId = Tuple.Key.ToString();
		FString FullCmd = FString::Printf(TEXT("MSaveManager.Load %s"), *SaveId);
		FString FullCmdRaw = FString::Printf(TEXT("MSaveManager.LoadRaw %s"), *SaveId);

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
	// #endif
}

void UMSaveManager::ConsoleSaveGame()
{
	// #if UE_WITH_CHEAT_MANAGER
	UMSaveNode* SaveNode = SaveGame();

	if (!GEngine) return;

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
	// #endif
}

void UMSaveManager::ConsoleLoadGame(const TArray<FString>& Args)
{
	// #if UE_WITH_CHEAT_MANAGER
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

	// Load and create a new invisible node
	UMSaveNode* SaveNode = LoadGame(SaveId, false);

	if (!GEngine) return;

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
	// #endif
}

void UMSaveManager::ConsoleLoadGameRaw(const TArray<FString>& Args)
{
	// #if UE_WITH_CHEAT_MANAGER
	if (Args.Num() != 1)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.LoadRaw <SaveId>"));
		return;
	}

	FGuid SaveId;
	if (!FGuid::Parse(Args[0], SaveId))
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, FString::Printf(TEXT("Invalid Guid: %s"), *Args[0]));
		return;
	}

	LoadGame(SaveId, true); // Load raw

	if (!GEngine) return;
	// TODO: LoadGame raw should better signify whether it was successful
	bool bSuccessful = SaveId == ActiveSaveGame->MostRecentNodeId;

	if (bSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			5.0f,
			FColor::Green,
			FString::Printf(
				TEXT("Loaded save - %s:%d (%s)"),
				*ActiveSaveGame->SlotName,
				ActiveSaveGame->UserIndex,
				*SaveId.ToString()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			0, 5.0f, FColor::Red, FString::Printf(TEXT("Failed to load save - %s (Does it exist?)"), *Args[0]));
	}

	// #endif
}

void UMSaveManager::ConsoleCreateSaveSlot(const TArray<FString>& Args)
{
	// #if UE_WITH_CHEAT_MANAGER
	if (Args.Num() != 2)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.SaveSlot <SlotName> <UserIndex>"));
		return;
	}

	UMSaveGame* SaveGame = CreateSaveSlot(Args[0], FCString::Atoi(*Args[1]));

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
	// #endif
}

void UMSaveManager::ConsoleLoadSaveSlot(const TArray<FString>& Args)
{
	// #if UE_WITH_CHEAT_MANAGER
	if (Args.Num() != 2)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.LoadSlot <SlotName> <UserIndex>"));
		return;
	}

	UMSaveGame* SaveGame = LoadSaveSlot(Args[0], FCString::Atoi(*Args[1]));

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
	// #endif
}

void UMSaveManager::ConsoleDeleteSaveSlot(const TArray<FString>& Args)
{
	// #if UE_WITH_CHEAT_MANAGER
	if (Args.Num() != 2)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				0, 5.0f, FColor::Red, TEXT("Usage: MSaveSystem.DeleteSlot <SlotName> <UserIndex>"));
		return;
	}

	bool bSuccess = DeleteSaveSlot(Args[0], FCString::Atoi(*Args[1]));

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
	// #endif
}
