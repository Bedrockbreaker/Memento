// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "CoreMinimal.h"

#include "MemPrefixAssetFilter.generated.h"

USTRUCT(BlueprintType)
struct FMemPrefixAssetFilter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TArray<TSubclassOf<UObject>> AssetTypes;
};
