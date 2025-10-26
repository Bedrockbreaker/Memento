// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoSaveSystemDeveloper.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"
#include "SaveSystem/MSaveManager.h"

IMPLEMENT_MODULE(FMementoSaveSystemDeveloperModule, MementoSaveSystemDeveloper);

void FMementoSaveSystemDeveloperModule::StartupModule()
{
	FWorldDelegates::OnPostWorldInitialization.AddLambda(
		[this](UWorld* World, const UWorld::InitializationValues& Values) -> void {
			UGameInstance* GameInstance = World->GetGameInstance();
			if (!GameInstance) return;
			UMSaveManager* SaveManager = GameInstance->GetSubsystem<UMSaveManager>();
			if (!SaveManager) return;
			SaveManagerDebug = NewObject<UMSaveManagerDebug>(SaveManager);
			SaveManagerDebug->Initialize(SaveManager);
		});
}

void FMementoSaveSystemDeveloperModule::ShutdownModule()
{
	if (!SaveManagerDebug) return;
	SaveManagerDebug->Deinitialize();
	SaveManagerDebug = nullptr;
}
