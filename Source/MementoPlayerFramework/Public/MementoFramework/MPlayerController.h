// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "GameFramework/PlayerController.h"
#include "SaveSystem/IMSaveable.h"

#include "MPlayerController.generated.h"

class UInputMappingContext;

UCLASS(abstract)
class MEMENTOPLAYERFRAMEWORK_API AMPlayerController : public APlayerController, public IMSaveable
{
	GENERATED_BODY()

public:
	AMPlayerController();

protected:
	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Setup input mapping contexts */
	virtual void SetupInputComponent() override;

private:
	/** Consistent save id */
	virtual FString GetSaveId_Implementation() const override { return TEXT("TheStrangerController"); }

	/** Serialize the ControlRotation */
	virtual bool RequiresCustomSerialization() const override { return true; }

	/** Save/Load */
	virtual void Save(FArchive& OutData, bool bRecall, UMSaveHistory* SaveHistory) override;
	virtual void Load(FArchive& InData, bool bRecall, UMSaveHistory* SaveHistory) override;
};
