// Copyright © Bedrockbreaker 2025. MIT License

#include "Core/MementoGameMode.h"
#include "Core/MementoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMementoGameMode::AMementoGameMode()
{
	DefaultPawnClass = AMementoCharacter::StaticClass();
}
