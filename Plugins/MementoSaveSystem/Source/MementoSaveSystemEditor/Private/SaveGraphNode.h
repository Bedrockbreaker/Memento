// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "CoreMinimal.h"

/** Save Graph Node representation for rendering */
struct FMSaveGraphNode
{
public:
	/** The id of the save node */
	FGuid SaveId;

	/** The label of the node */
	FString Label;

	/** The calculated position based on row and column */
	FVector2f Position;

	/** Whether the node is currently selected (highlighted) */
	bool bSelected = false;

	/** Whether the node is currently invisible (non-filled, border only) */
	bool bInvisible = false;

	/** The direct timeline children of the node */
	TArray<FGuid> BranchChildren;

	/** The player sequence children of the node */
	TArray<FGuid> SequenceChildren;
};
