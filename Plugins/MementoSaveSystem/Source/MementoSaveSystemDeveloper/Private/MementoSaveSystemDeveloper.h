// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "SaveSystem/MSaveManagerDebug.h"

class FMementoSaveSystemDeveloperModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	UMSaveManagerDebug* SaveManagerDebug;
};
