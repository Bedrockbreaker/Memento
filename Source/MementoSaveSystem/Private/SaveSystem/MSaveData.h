// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "MSaveData.generated.h"

/** Simple container for binary save data */
USTRUCT(BlueprintType)
struct FMSaveData
{
	GENERATED_BODY()

public:
	/**
	 * The class name for the actor which this data is for.
	 * Used for runtime creation of actors initially missing from the map
	 */
	UPROPERTY(BlueprintReadWrite)
	FString ClassName;

	/** Raw binary blob */
	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> Data;
};
