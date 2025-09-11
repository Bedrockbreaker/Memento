// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoEditor.h"
#include "Modules/ModuleManager.h"

class FMementoEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FMementoEditorModule, MementoEditor)
