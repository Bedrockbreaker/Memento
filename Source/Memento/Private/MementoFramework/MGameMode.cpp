// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoFramework/MGameMode.h"

#include "Engine/GameInstance.h"
#include "MementoFramework/MCharacter.h"
#include "SaveSystem/MSaveGame.h"
#include "SaveSystem/MSaveManager.h"

AMGameMode::AMGameMode()
{
	DefaultPawnClass = AMCharacter::StaticClass();
}

void AMGameMode::StartPlay()
{
	Super::StartPlay();

#if WITH_EDITOR
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UMSaveManager* SaveManager = GameInstance->GetSubsystem<UMSaveManager>();
	if (!SaveManager || SaveManager->GetActiveSaveGame()) return;

	SaveManager->LoadOrCreateSaveSlot(TEXT("TestSlot"), 0);
	SaveManager->LoadGame(SaveManager->GetActiveSaveGame()->MostRecentNodeId);
#endif
}
