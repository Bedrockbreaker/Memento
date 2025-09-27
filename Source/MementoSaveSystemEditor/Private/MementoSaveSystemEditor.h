// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;
class UMSaveGame;
class SMSaveGraph;
class FSpawnTabArgs;

/** Memento Save System Editor Module */
class FMementoSaveSystemEditorModule : public IModuleInterface
{
public:
	/** Module startup */
	virtual void StartupModule() override;

	/** Module shutdown */
	virtual void ShutdownModule() override;

private:
	/** Save Graph visualization widget */
	TSharedPtr<SMSaveGraph> SaveGraphWidget;

	/** Subscribe to UMSaveManager::OnSaveSlotUpdated */
	void OnPIEStarted(bool bIsSimulating);

	/** Unsubscribe from UMSaveManager::OnSaveSlotUpdated */
	void OnPIEEnded(bool bIsSimulating);

	/** Handle UMSaveManager::OnSaveSlotUpdated */
	void OnSaveSlotUpdated(UMSaveGame* SaveGame);

	/** Spawns the Save Graph visualization tab */
	TSharedRef<SDockTab> SpawnSaveGraphTab(const FSpawnTabArgs& SpawnTabArgs);
};
