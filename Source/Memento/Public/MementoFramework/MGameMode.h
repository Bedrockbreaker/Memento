// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "GameFramework/GameModeBase.h"

#include "MGameMode.generated.h"

/** Default game mode for Memento */
UCLASS(MinimalAPI)
class AMGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMGameMode();

	virtual void StartPlay() override;
};
