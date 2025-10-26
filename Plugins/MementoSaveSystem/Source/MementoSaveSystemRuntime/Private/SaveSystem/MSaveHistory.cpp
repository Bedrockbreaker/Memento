// Copyright © Bedrockbreaker 2025. MIT License

#include "SaveSystem/MSaveHistory.h"

#include "Kismet/GameplayStatics.h"
#include "SaveSystem/MSaveData.h"
#include "SaveSystem/MSaveGame.h"
#include "SaveSystem/MSaveNode.h"

void UMSaveHistory::Initialize(UMSaveGame* InSaveGame)
{
	SaveGame = InSaveGame;
	if (SaveGame)
	{
		LoadAllNodes();
	}
	else
	{
		SaveNodes.Reset();
	}
}

bool UMSaveHistory::GetLastSaveState(const FString& SaveableId, FMSaveData& OutSaveData) const
{
	return GetNthLastSaveState(SaveableId, 1, OutSaveData);
}

bool UMSaveHistory::GetNthLastSaveState(const FString& SaveableId, int32 N, FMSaveData& OutSaveData) const
{
	checkf(N > 0, TEXT("N must be greater than 0."));
	if (!SaveGame || SaveNodes.IsEmpty()) return false;

	FGuid SaveNodeId = SaveGame->MostRecentNodeId;
	if (!SaveNodeId.IsValid()) return false;

	for (int32 Index = 0; Index < N; ++Index)
	{
		if (!SaveNodes.Contains(SaveNodeId)) return false;
		SaveNodeId = SaveGame->SaveNodes[SaveNodeId].SequenceParentId;
	}

	UMSaveNode* SaveNode = SaveNodes.FindChecked(SaveNodeId);
	if (!SaveNode->SaveData.Contains(SaveableId)) return false;

	OutSaveData = SaveNode->SaveData[SaveableId];
	return true;
}

bool UMSaveHistory::GetAllSaveStates(const FString& SaveableId, TArray<FMSaveData>& OutSaveData) const
{
	return false;
}

int32 UMSaveHistory::GetDistinctStateCount(const FString& SaveableId) const
{
	return 0;
}

int32 UMSaveHistory::GetBranchChildrenCount(const FGuid& SaveNodeId) const
{
	return 0;
}

void UMSaveHistory::LoadAllNodes()
{
	SaveNodes.Reset();

	if (!SaveGame) return;

	for (const TTuple<FGuid, FMSaveNodeMetadata>& Node : SaveGame->SaveNodes)
	{
		FString		SlotName = SaveGame->SlotName + Node.Key.ToString();
		UMSaveNode* SaveNode = Cast<UMSaveNode>(UGameplayStatics::LoadGameFromSlot(SlotName, SaveGame->UserIndex));

		if (!SaveNode) continue;

		SaveNodes.Add(Node.Key, SaveNode);
	}
}
