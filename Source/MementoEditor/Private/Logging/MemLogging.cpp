// Copyright © Bedrockbreaker 2025. MIT License

#include "MemLogging.h"

#include "Logging/LogMacros.h"
#include "MemLogVerbosityLevel.h"

DEFINE_LOG_CATEGORY(LogBlueprintLogMessage);

void UMemLogging::LogMessage(const FString& Message, EMemLogVerbosityLevel Verbosity)
{
	switch (Verbosity)
	{
		case EMemLogVerbosityLevel::VeryVerbose:
			UE_LOG(LogBlueprintLogMessage, VeryVerbose, TEXT("%s"), *Message);
			break;
		case EMemLogVerbosityLevel::Verbose:
			UE_LOG(LogBlueprintLogMessage, Verbose, TEXT("%s"), *Message);
			break;
		case EMemLogVerbosityLevel::Log:
			UE_LOG(LogBlueprintLogMessage, Log, TEXT("%s"), *Message);
			break;
		case EMemLogVerbosityLevel::Warning:
			UE_LOG(LogBlueprintLogMessage, Warning, TEXT("%s"), *Message);
			break;
		case EMemLogVerbosityLevel::Error:
			UE_LOG(LogBlueprintLogMessage, Error, TEXT("%s"), *Message);
			break;
		case EMemLogVerbosityLevel::Fatal:
			UE_LOG(LogBlueprintLogMessage, Fatal, TEXT("%s"), *Message);
			break;
	}
}
