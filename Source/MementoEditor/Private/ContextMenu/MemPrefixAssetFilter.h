// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "MemPrefixAssetFilter.generated.h"

USTRUCT(BlueprintType)
struct FMemPrefixAssetFilter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TArray<TSubclassOf<UObject>> AssetTypes;
};
