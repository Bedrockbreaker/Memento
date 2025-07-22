// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "ActorActionUtility.h"
#include "CoreMinimal.h"

#include "ActorHelloWorld.generated.h"

UCLASS()
class UMemActorHelloWorld : public UActorActionUtility
{
	GENERATED_BODY()

public:
	UFUNCTION(CallInEditor)
	void PrintActorNames() const;
};
