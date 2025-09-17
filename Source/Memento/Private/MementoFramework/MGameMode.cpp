// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoFramework/MGameMode.h"

#include "MementoFramework/MCharacter.h"

AMGameMode::AMGameMode()
{
	DefaultPawnClass = AMCharacter::StaticClass();
}
