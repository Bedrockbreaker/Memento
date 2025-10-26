// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoFramework/MPlayerController.h"

#include "Camera/MPlayerCameraManager.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AMPlayerController::AMPlayerController()
{
	PlayerCameraManagerClass = AMPlayerCameraManager::StaticClass();
}

void AMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController()) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
	{
		Subsystem->AddMappingContext(CurrentContext, 0);
	}
}

void AMPlayerController::Save(FArchive& OutData, bool bRecall, UMSaveHistory* SaveHistory)
{
	OutData << ControlRotation;
}

void AMPlayerController::Load(FArchive& InData, bool bRecall, UMSaveHistory* SaveHistory)
{
	InData << ControlRotation;
}
