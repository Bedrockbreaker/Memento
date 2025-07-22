// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "UtilNodes.generated.h"

UCLASS()
class UMemUtilNodes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Development", meta = (DevelopmentOnly))
	static void HelloWorld();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Vector")
	static FVector VectorBetween(const FVector& Start, const FVector& End);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Random")
	static float RandomGaussian(float Mean = 0.0f, float StandardDeviation = 1.0f);
};
