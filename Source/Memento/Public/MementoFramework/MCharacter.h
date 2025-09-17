// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "GameFramework/Character.h"

#include "MCharacter.generated.h"

MEMENTO_API DECLARE_LOG_CATEGORY_EXTERN(LogMCharacter, Log, All);

/**
 * Base character for Memento. Handles character movement, etc.
 * Knows nothing about player input or AI things.
 */
UCLASS(abstract)
class MEMENTO_API AMCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMCharacter();
};
