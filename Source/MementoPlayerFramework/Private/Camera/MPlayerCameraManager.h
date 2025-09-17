// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "Camera/PlayerCameraManager.h"

#include "MPlayerCameraManager.generated.h"

/** First person camera manager; limits minimum/maximum pitch */
UCLASS()
class AMPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AMPlayerCameraManager();
};
