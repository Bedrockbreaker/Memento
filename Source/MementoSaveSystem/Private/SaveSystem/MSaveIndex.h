// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "GameFramework/SaveGame.h"

#include "MSaveIndex.generated.h"

struct FMSlotId;

/** A list of all known save slots */
UCLASS()
class UMSaveIndex : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	/** A list of all save slots */
	TArray<FMSlotId> SaveSlots;
};
