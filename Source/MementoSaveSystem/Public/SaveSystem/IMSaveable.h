// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "UObject/Interface.h"

#include "IMSaveable.generated.h"

/** Internal interface for saveable actors */
UINTERFACE(MinimalAPI)
class UMSaveable : public UInterface
{
	GENERATED_BODY()
};

/** Interface for saveable actors */
class MEMENTOSAVESYSTEM_API IMSaveable
{
	GENERATED_BODY()

public:
	/** The stable save id for this actor */
	virtual FString GetSaveId() const = 0;

	/**
	 * If returning false, the Save and Load functions will not be called,
	 * and only properties marked as SaveGame will be serialized.
	 * SaveGame properties will still be automatically serialized if this returns true.
	 */
	virtual bool RequiresCustomSerialization() const { return false; }

	/** Custom serialization for this actor */
	virtual void Save(FArchive& OutData) {}

	/** Custom deserialization for this actor */
	virtual void Load(FArchive& InData) {}
};
