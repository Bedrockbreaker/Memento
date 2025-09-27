// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "SaveGraphNode.h"
#include "Widgets/SCompoundWidget.h"

class UMSaveGame;

/** Save Graph Widget */
class SMSaveGraph : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMSaveGraph) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Refresh Graph internals */
	void RefreshGraph(UMSaveGame* SaveGame);

private:
	/** Node size */
	FVector2f NodeSize = FVector2f(120.0f, 40.0f);

	/** Gap spacing between nodes */
	FVector2f Spacing = FVector2f(20.0f, 20.0f);

	/** Maximum width of the graph, based on the number of columns */
	float MaximumWidth = 0.0f;

	/** Maximum height of the graph, based on the number of rows */
	float MaximumHeight = 0.0f;

	/** The label of the save slot (SlotName:UserIndex) */
	FString SaveSlotLabel = TEXT("No save slot loaded");

	/** Lighter-weight representation of the nodes contained in a UMSaveGame */
	TMap<FGuid, FMSaveGraphNode> SaveGraphNodes;

	/** Display the Save Graph */
	virtual int32 OnPaint(
		const FPaintArgs&		 Args,
		const FGeometry&		 AllottedGeometry,
		const FSlateRect&		 MyClippingRect,
		FSlateWindowElementList& OutDrawElements,
		int32					 LayerId,
		const FWidgetStyle&		 InWidgetStyle,
		bool					 bParentEnabled) const override;

	/** The desired size underneath the SScrollBox */
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
};
