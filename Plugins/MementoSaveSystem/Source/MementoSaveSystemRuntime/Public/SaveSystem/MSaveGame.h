// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "GameFramework/SaveGame.h"
#include "SaveSystem/MSaveNodeMetadata.h"

#include "MSaveGame.generated.h"

/** Container for save game data within a single save slot */
UCLASS()
class MEMENTOSAVESYSTEMRUNTIME_API UMSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** The name of the save slot */
	UPROPERTY(BlueprintReadOnly)
	FString SlotName;

	/** The user index of the save slot */
	UPROPERTY(BlueprintReadOnly)
	int32 UserIndex = 0;

	/** A DAG of metadata for each save node, to avoid unncessary deserialization during graph traversal */
	UPROPERTY()
	TMap<FGuid, FMSaveNodeMetadata> SaveNodes;

	/** The id of the most recent save node added to the save graph */
	UPROPERTY(BlueprintReadOnly)
	FGuid MostRecentNodeId;
};
