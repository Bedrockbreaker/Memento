// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "MSlotId.generated.h"

/** A simple struct to identify a save slot */
USTRUCT(BlueprintType)
struct MEMENTOSAVESYSTEM_API FMSlotId
{
	GENERATED_BODY()

public:
	/** The name of the save slot */
	UPROPERTY(BlueprintReadOnly)
	FString SlotName;

	/** The user index of the save slot */
	UPROPERTY(BlueprintReadOnly)
	int32 UserIndex = 0;

	/** Equality operator */
	bool operator==(const FMSlotId& Other) const { return SlotName == Other.SlotName && UserIndex == Other.UserIndex; }
};
