// Copyright © Bedrockbreaker 2025. MIT License

#include "UtilNodes.h"

#include "Engine/Engine.h"

void UMemUtilNodes::HelloWorld()
{
#if WITH_EDITOR
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Hello World!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello World!"));
	}
#endif
}

FVector UMemUtilNodes::VectorBetween(const FVector& Start, const FVector& End)
{
	return End - Start;
}

float UMemUtilNodes::RandomGaussian(float Mean, float StandardDeviation)
{
	// Box-Muller transform, not optimized
	float U = FMath::FRand();
	float V = FMath::FRand();
	return FMath::Sqrt(-2 * FMath::Loge(U)) * FMath::Cos(2 * UE_PI * V) * StandardDeviation + Mean;
}
