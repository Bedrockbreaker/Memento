// Copyright © Bedrockbreaker 2025. MIT License

#include "SaveSystem/MSaveManager.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/IMSaveable.h"
#include "SaveSystem/MSaveData.h"
#include "SaveSystem/MSaveGame.h"
#include "SaveSystem/MSaveHistory.h"
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

UMSaveNode* UMSaveManager::SaveGame(bool bInvisible)
{
	checkf(ActiveSaveGame, TEXT("A save slot must be created/loaded before saving."));

	UMSaveNode* SaveNode = CreateSaveNode(
		ActiveSaveGame->MostRecentNodeId, ActiveSaveGame->MostRecentNodeId, /* bRecall = */ false, bInvisible);
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

void UMSaveManager::AsyncSaveGame(FMAsyncSaveGameDelegate Delegate, bool bInvisible)
{
	checkf(ActiveSaveGame, TEXT("A save slot must be created/loaded before saving."));

	UMSaveNode* SaveNode = CreateSaveNode(
		ActiveSaveGame->MostRecentNodeId, ActiveSaveGame->MostRecentNodeId, /** bRecall = */ false, bInvisible);
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

void UMSaveManager::AsyncSaveGameDynamic(FMAsyncSaveGameDelegateDynamic Delegate, bool bInvisible)
{
	FMAsyncSaveGameDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda(
			[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveNode* SaveNode) -> void {
				Delegate.ExecuteIfBound(SlotName, UserIndex, SaveNode);
			});
	}

	AsyncSaveGame(MoveTemp(NativeDelegate), bInvisible);
}

UMSaveNode* UMSaveManager::LoadGame(FGuid SaveId)
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

	bool bSuccess = LoadSaveNode(SaveNode, /** bRecall = */ false);
	if (bSuccess)
	{
		ActiveSaveGame->MostRecentNodeId = SaveId;
		OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
	}

	return SaveNode;
}

void UMSaveManager::AsyncLoadGame(FMAsyncLoadGameDelegate Delegate, FGuid SaveId)
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
		[Delegate, SaveId, this](const FString& InSlotName, const int32 InUserIndex, USaveGame* SaveNode) -> void {
			UMSaveNode* MSaveNode = Cast<UMSaveNode>(SaveNode);
			bool		bSuccess = MSaveNode != nullptr;

			if (!bSuccess)
			{
				Delegate.ExecuteIfBound(InSlotName, InUserIndex, MSaveNode);
				return;
			}

			bSuccess = LoadSaveNode(MSaveNode, /** bRecall = */ false);
			if (bSuccess)
			{
				ActiveSaveGame->MostRecentNodeId = SaveId;
				OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
			}

			Delegate.ExecuteIfBound(InSlotName, InUserIndex, MSaveNode);
		});

	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, ActiveSaveGame->UserIndex, MoveTemp(LoadDelegate));
}

void UMSaveManager::AsyncLoadGameDynamic(FMAsyncLoadGameDelegateDynamic Delegate, FGuid SaveId)
{
	FMAsyncLoadGameDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda(
			[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveNode* SaveNode) -> void {
				Delegate.ExecuteIfBound(SlotName, UserIndex, SaveNode);
			});
	}

	AsyncLoadGame(MoveTemp(NativeDelegate), SaveId);
}

UMSaveNode* UMSaveManager::RecallGame(FGuid BranchParentId, FGuid SequenceParentId, bool bInvisible)
{
	checkf(ActiveSaveGame, TEXT("A save slot must be created/loaded before recalling."));

	// 1. Load save objects

	FString		BranchSlotName = ActiveSaveGame->SlotName + BranchParentId.ToString();
	UMSaveNode* BranchNode =
		Cast<UMSaveNode>(UGameplayStatics::LoadGameFromSlot(BranchSlotName, ActiveSaveGame->UserIndex));

	bool bSuccess = LoadSaveNode(BranchNode, /** bRecall = */ true);
	if (!bSuccess) return nullptr;

	// 2. Save save objects
	SequenceParentId = SequenceParentId.IsValid() ? SequenceParentId : ActiveSaveGame->MostRecentNodeId;
	UMSaveNode* SaveNode = CreateSaveNode(BranchParentId, SequenceParentId, /* bRecall = */ true, bInvisible);
	if (!SaveNode) return nullptr;

	UE_LOG(
		LogMSaveManager,
		Log,
		TEXT("Recalling game - %s:%d (%s)"),
		*ActiveSaveGame->SlotName,
		ActiveSaveGame->UserIndex,
		*SaveNode->SaveId.ToString());

	FString SlotName = ActiveSaveGame->SlotName + SaveNode->SaveId.ToString();

	bSuccess = UGameplayStatics::SaveGameToSlot(SaveNode, SlotName, ActiveSaveGame->UserIndex);
	bSuccess = bSuccess
		&& UGameplayStatics::SaveGameToSlot(ActiveSaveGame, ActiveSaveGame->SlotName, ActiveSaveGame->UserIndex);

	if (bSuccess)
	{
		ActiveSaveGame->MostRecentNodeId = SaveNode->SaveId;
		OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
	}

	return bSuccess ? SaveNode : nullptr;
}

void UMSaveManager::AsyncRecallGame(
	FMAsyncRecallGameDelegate Delegate, FGuid BranchParentId, FGuid SequenceParentId, bool bInvisible)
{
	checkf(ActiveSaveGame, TEXT("A save slot must be created/loaded before recalling."));

	// 1. Load save objects

	FString BranchSlotName = ActiveSaveGame->SlotName + BranchParentId.ToString();

	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindLambda(
		[Delegate, BranchParentId, bInvisible, this](
			const FString& InSlotName, const int32 InUserIndex, USaveGame* BranchParent) -> void {
			UMSaveNode* MBranchParent = Cast<UMSaveNode>(BranchParent);

			bool bSuccess = LoadSaveNode(MBranchParent, /** bRecall = */ true);
			if (!bSuccess)
			{
				Delegate.ExecuteIfBound(InSlotName, InUserIndex, nullptr);
				return;
			}

			// 2. Save save objects

			UMSaveNode* SaveNode = CreateSaveNode(
				ActiveSaveGame->MostRecentNodeId, ActiveSaveGame->MostRecentNodeId, /** bRecall = */ true, bInvisible);
			if (!SaveNode)
			{
				Delegate.ExecuteIfBound(ActiveSaveGame->SlotName, ActiveSaveGame->UserIndex, nullptr);
				return;
			}

			FAsyncSaveGameToSlotDelegate SaveDelegate;
			SaveDelegate.BindLambda(
				[Delegate, SaveNode, this](const FString& SlotName, const int32 UserIndex, bool bSuccess) -> void {
					if (bSuccess)
					{
						ActiveSaveGame->MostRecentNodeId = SaveNode->SaveId;
						OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
					}
					Delegate.ExecuteIfBound(SlotName, UserIndex, bSuccess ? SaveNode : nullptr);
				});

			UE_LOG(
				LogMSaveManager,
				Log,
				TEXT("Recalling game - %s:%d (%s)"),
				*ActiveSaveGame->SlotName,
				ActiveSaveGame->UserIndex,
				*SaveNode->SaveId.ToString());

			FString SlotName = ActiveSaveGame->SlotName + SaveNode->SaveId.ToString();

			UGameplayStatics::AsyncSaveGameToSlot(
				SaveNode, SlotName, ActiveSaveGame->UserIndex, MoveTemp(SaveDelegate));
			// TODO: the slot save also needs to be factored in for returning success
			UGameplayStatics::AsyncSaveGameToSlot(
				ActiveSaveGame, ActiveSaveGame->SlotName, ActiveSaveGame->UserIndex, nullptr);
		});

	UGameplayStatics::AsyncLoadGameFromSlot(BranchSlotName, ActiveSaveGame->UserIndex, MoveTemp(LoadDelegate));
}

void UMSaveManager::AsyncRecallGameDynamic(
	FMAsyncRecallGameDelegateDynamic Delegate, FGuid BranchParentId, FGuid SequenceParentId, bool bInvisible)
{
	FMAsyncRecallGameDelegate NativeDelegate;
	if (Delegate.IsBound())
	{
		NativeDelegate.BindLambda(
			[Delegate](const FString& SlotName, const int32 UserIndex, UMSaveNode* SaveNode) -> void {
				Delegate.ExecuteIfBound(SlotName, UserIndex, SaveNode);
			});
	}

	AsyncRecallGame(MoveTemp(NativeDelegate), BranchParentId, SequenceParentId, bInvisible);
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
		OnSaveIndexUpdated.Broadcast(SaveIndex);
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
							if (bSuccess)
							{
								ActiveSaveGame = SaveGame;
								OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
								OnSaveIndexUpdated.Broadcast(SaveIndex);
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
		SaveHistory->Initialize(SaveGame);
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
				SaveHistory->Initialize(MSaveGame);
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
		SaveHistory->Initialize(nullptr);
		OnSaveSlotUpdated.Broadcast(ActiveSaveGame);
	}

	SaveIndex->SaveSlots.Remove({ SlotName, UserIndex });
	UGameplayStatics::SaveGameToSlot(SaveIndex, TEXT("SaveIndex"), 0);

	OnSaveIndexUpdated.Broadcast(SaveIndex);

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
				SaveHistory->Initialize(nullptr);
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

UMSaveGame* UMSaveManager::CloneSaveSlot(
	const FString& OriginalSlotName,
	const int32	   OriginalUserIndex,
	const FString& NewSlotName,
	const int32	   NewUserIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(OriginalSlotName, OriginalUserIndex)) return nullptr;

	UE_LOG(
		LogMSaveManager,
		Log,
		TEXT("Cloning save slot - %s:%d -> %s:%d"),
		*OriginalSlotName,
		OriginalUserIndex,
		*NewSlotName,
		NewUserIndex);

	UMSaveGame* OriginalSaveGame = LoadSaveSlot(OriginalSlotName, OriginalUserIndex, false);
	if (!OriginalSaveGame) return nullptr;

	UMSaveGame* NewSaveGame = CreateSaveSlot(NewSlotName, NewUserIndex);
	if (!NewSaveGame) return nullptr;

	NewSaveGame->SlotName = NewSlotName;
	NewSaveGame->UserIndex = NewUserIndex;
	NewSaveGame->MostRecentNodeId = OriginalSaveGame->MostRecentNodeId;

	bool bSuccess = true;

	for (const TTuple<FGuid, FMSaveNodeMetadata>& OriginalMetadata : OriginalSaveGame->SaveNodes)
	{
		FString		OriginalNodeSlotName = OriginalSaveGame->SlotName + OriginalMetadata.Key.ToString();
		UMSaveNode* OriginalSaveNode =
			Cast<UMSaveNode>(UGameplayStatics::LoadGameFromSlot(OriginalNodeSlotName, OriginalSaveGame->UserIndex));
		UMSaveNode* NewSaveNode = CloneSaveNode(OriginalSaveNode);
		if (!NewSaveNode) return nullptr;

		FString NewNodeSlotName = NewSlotName + NewSaveNode->SaveId.ToString();
		bSuccess = bSuccess && UGameplayStatics::SaveGameToSlot(NewSaveNode, NewNodeSlotName, NewSaveGame->UserIndex);
		if (!bSuccess) return nullptr;

		NewSaveGame->SaveNodes.Add(OriginalMetadata.Key, OriginalMetadata.Value);
	}

	bSuccess = bSuccess && UGameplayStatics::SaveGameToSlot(NewSaveGame, NewSlotName, NewSaveGame->UserIndex);

	return bSuccess ? NewSaveGame : nullptr;
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
				}
				else
				{
					UE_LOG(LogMSaveManager, Warning, TEXT("Failed to load SaveIndex"));
				}
			});

		UGameplayStatics::AsyncLoadGameFromSlot(TEXT("SaveIndex"), 0, MoveTemp(Delegate));
	}

	SaveHistory = NewObject<UMSaveHistory>(this);
	SaveHistory->Initialize(ActiveSaveGame);
}

void UMSaveManager::Deinitialize()
{
	OnSaveSlotUpdated.RemoveAll(this);

	Super::Deinitialize();
}

void UMSaveManager::FindSaveables(TArray<UObject*>& OutSaveables) const
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (Actor->IsPendingKillPending()) continue;

		if (Actor->Implements<UMSaveable>()) OutSaveables.Add(Actor);

		// Components may also be saveable
		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (Component->Implements<UMSaveable>()) OutSaveables.Add(Component);
		}
	}
}

UMSaveNode* UMSaveManager::CreateSaveNode(FGuid BranchParentId, FGuid SequenceParentId, bool bRecall, bool bInvisible)
{
	FGuid SaveId = FGuid::NewGuid();

	UMSaveNode* SaveNode = Cast<UMSaveNode>(UGameplayStatics::CreateSaveGameObject(UMSaveNode::StaticClass()));
	SaveNode->SaveId = SaveId;

	FMSaveNodeMetadata Metadata;
	Metadata.SaveId = SaveId;
	Metadata.BranchParentId = BranchParentId.IsValid() ? BranchParentId : ActiveSaveGame->MostRecentNodeId;
	Metadata.SequenceParentId = SequenceParentId.IsValid() ? SequenceParentId : ActiveSaveGame->MostRecentNodeId;
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

	TArray<UObject*> Saveables;
	FindSaveables(Saveables);

	for (UObject* Saveable : Saveables)
	{
		if (!Saveable) continue;

		FMSaveData SaveData;
		SaveData.ClassName = Saveable->GetClass()->GetPathName();
		SaveData.ActorFName = Saveable->GetFName();

		AActor* Actor = Cast<AActor>(Saveable);
		if (Actor) SaveData.Transform = Actor->GetActorTransform();

		FMemoryWriter					   Writer(SaveData.Data, true);
		FObjectAndNameAsStringProxyArchive Archive(Writer, true);
		Archive.ArIsSaveGame = true; // Serialize all properties marked as SaveGame
		Archive.ArNoDelta = true;	 // Blueprint properties don't serialize consistently without this
		Saveable->Serialize(Archive);

		IMSaveable* NativeSaveable = Cast<IMSaveable>(Saveable);
		if (NativeSaveable && NativeSaveable->RequiresCustomSerialization())
			NativeSaveable->Save(Writer, bRecall, SaveHistory);

		SaveNode->SaveData.Add(IMSaveable::Execute_GetSaveId(Saveable), MoveTemp(SaveData));
	}

	return SaveNode;
}

UMSaveNode* UMSaveManager::CloneSaveNode(const UMSaveNode* OriginalSaveNode)
{
	if (!OriginalSaveNode) return nullptr;

	UMSaveNode* SaveNode = Cast<UMSaveNode>(UGameplayStatics::CreateSaveGameObject(UMSaveNode::StaticClass()));
	SaveNode->SaveId = OriginalSaveNode->SaveId;
	SaveNode->SaveData = OriginalSaveNode->SaveData;

	return SaveNode;
}

bool UMSaveManager::LoadSaveNode(UMSaveNode* SaveNode, bool bRecall)
{
	if (!SaveNode) return false;

	TArray<UObject*> Saveables;
	FindSaveables(Saveables);

	for (UObject* Saveable : Saveables)
	{
		if (!Saveable) continue;

		// TODO: Store a hash map of SaveId -> Saveable to avoid O(n^2) search
		FMSaveData* SaveData = SaveNode->SaveData.Find(IMSaveable::Execute_GetSaveId(Saveable));
		// TODO: create runtime-generated objects if they don't exist
		if (!SaveData) continue;

		UE_LOG(LogMSaveManager, Log, TEXT("  Loading saveable - %s"), *Saveable->GetName());

		AActor* Actor = Cast<AActor>(Saveable);
		if (Actor) Actor->SetActorTransform(SaveData->Transform);

		FMemoryReader					   Reader(SaveData->Data, true);
		FObjectAndNameAsStringProxyArchive Archive(Reader, true);
		Archive.ArIsSaveGame = true; // Serialize all properties marked as SaveGame
		Archive.ArNoDelta = true;	 // Blueprint properties don't serialize consistently without this
		Saveable->Serialize(Archive);

		IMSaveable* NativeSaveable = Cast<IMSaveable>(Saveable);
		if (NativeSaveable && NativeSaveable->RequiresCustomSerialization())
			NativeSaveable->Load(Reader, bRecall, SaveHistory);
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
