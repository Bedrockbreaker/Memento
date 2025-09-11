// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoDevelopment.h"

#include "Modules/ModuleManager.h"

class FMementoDevelopmentModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FMementoDevelopmentModule, MementoDevelopment)
