// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/LogMacros.h"
#include "MemLogVerbosityLevel.h"

#include "MemLogging.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBlueprintLogMessage, Log, All);

UCLASS()
class UMemLogging : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Utilities|String", meta = (DevelopmentOnly))
	static void LogMessage(const FString& Message, EMemLogVerbosityLevel Verbosity = EMemLogVerbosityLevel::Log);
};
