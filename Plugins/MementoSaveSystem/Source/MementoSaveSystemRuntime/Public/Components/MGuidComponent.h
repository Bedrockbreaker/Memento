// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "Components/ActorComponent.h"

#include "MGuidComponent.generated.h"

/** Provides a stable guid for an actor, across both editor and runtime */
UCLASS(meta = (BlueprintSpawnableComponent))
class MEMENTOSAVESYSTEMRUNTIME_API UMGuidComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGuidComponent();

	/** Returns the guid */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FGuid GetGuid() const { return Guid; }

	/** Returns the guid as a string */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetString() const { return Guid.ToString(); }

#if WITH_EDITORONLY_DATA
	/** Create a new guid when the component is created */
	virtual void OnComponentCreated() override;

	/** Create a new guid when the component is loaded and the guid is invalid */
	virtual void PostLoad() override;

	/** Create a new guid when the component is duplicated */
	virtual void PostEditImport() override;
#endif

private:
	/** The internal guid */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	FGuid Guid;
};
