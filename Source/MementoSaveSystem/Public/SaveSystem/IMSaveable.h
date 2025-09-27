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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MSave System")
	FString GetSaveId() const;

	/** The stable save id for this actor. This is required to be overridden. */
	virtual FString GetSaveId_Implementation() const
	{
		checkf(false, TEXT("GetSaveId_Implementation must be overridden. (Or don't call the parent function)"));
		return TEXT("");
	}

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
