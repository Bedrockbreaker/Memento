// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "CoreMinimal.h"

#include "MemLogVerbosityLevel.generated.h"

UENUM(BlueprintType)
enum class EMemLogVerbosityLevel : uint8
{
	VeryVerbose UMETA(DisplayName = "VeryVerbose"),
	Verbose		UMETA(DisplayName = "Verbose"),
	Log			UMETA(DisplayName = "Log"),
	Warning		UMETA(DisplayName = "Warning"),
	Error		UMETA(DisplayName = "Error"),
	Fatal		UMETA(DisplayName = "Fatal")
};
