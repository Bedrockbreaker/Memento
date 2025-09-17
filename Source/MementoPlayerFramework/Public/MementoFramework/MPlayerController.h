// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "GameFramework/PlayerController.h"

#include "MPlayerController.generated.h"

class UInputMappingContext;

UCLASS(abstract)
class MEMENTOPLAYERFRAMEWORK_API AMPlayerController : public APlayerController
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
};
