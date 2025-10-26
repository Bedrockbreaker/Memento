// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "GameFramework/SaveGame.h"
#include "SaveSystem/MSaveData.h"

#include "MSaveNode.generated.h"

/** Container for save data within a single save node */
UCLASS(BlueprintType)
class MEMENTOSAVESYSTEMRUNTIME_API UMSaveNode : public USaveGame
{
	GENERATED_BODY()

public:
	/** The id of the save node */
	UPROPERTY(BlueprintReadOnly)
	FGuid SaveId;

	/** The save data, mapping actor save ids to save data */
	UPROPERTY()
	TMap<FString, FMSaveData> SaveData;
};
