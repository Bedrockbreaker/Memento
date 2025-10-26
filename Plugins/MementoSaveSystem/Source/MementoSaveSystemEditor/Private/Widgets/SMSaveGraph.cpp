// Copyright © Bedrockbreaker 2025. MIT License

#include "Widgets/SMSaveGraph.h"

#include "Containers/UnrealString.h"
#include "Editor.h"
#include "Rendering/SlateLayoutTransform.h"
#include "SaveGraphNode.h"
#include "SaveSystem/MSaveGame.h"
#include "Widgets/SNullWidget.h"

void SMSaveGraph::Construct(const FArguments& InArgs)
{
	// This widget cannot contain any children
	ChildSlot[SNullWidget::NullWidget];

	MaximumWidth = NodeSize.X;
	MaximumHeight = NodeSize.Y;
}

void SMSaveGraph::RefreshGraph(UMSaveGame* SaveGame)
{
	if (!SaveGame) return;

	SaveSlotLabel = FString::Printf(TEXT("%s:%d"), *SaveGame->SlotName, SaveGame->UserIndex);
	SaveGraphNodes.Reset();
	MaximumWidth = NodeSize.X;
	MaximumHeight = NodeSize.Y;

	if (SaveGame->SaveNodes.IsEmpty()) return;

	// 1. Construct a map of all children
	TMap<FGuid, TArray<FGuid>> BranchChildren;
	TMap<FGuid, TArray<FGuid>> SequenceChildren;
	FGuid					   RootNodeId;

	for (const TTuple<FGuid, FMSaveNodeMetadata>& SaveNode : SaveGame->SaveNodes)
	{
		if (!SaveNode.Value.BranchParentId.IsValid())
		{
			RootNodeId = SaveNode.Key;
			continue;
		}

		BranchChildren.FindOrAdd(SaveNode.Value.BranchParentId).Add(SaveNode.Key);
		SequenceChildren.FindOrAdd(SaveNode.Value.SequenceParentId).Add(SaveNode.Key);
	}

	// 2. Find the y position of all nodes, based on the depth of BranchChildren
	TQueue<FMSaveGraphNode> DepthQueue;
	TArray<FMSaveGraphNode> ColumnStack; // Create this ahead of time to save a time on iterations

	DepthQueue.Enqueue({
		RootNodeId,
		RootNodeId.ToString().Left(8),
		{ Spacing.X, Spacing.Y },
		RootNodeId == SaveGame->MostRecentNodeId,
		SaveGame->SaveNodes[RootNodeId].bInvisible,
		BranchChildren.FindRef(RootNodeId),
		SequenceChildren.FindRef(RootNodeId),
	});

	while (!DepthQueue.IsEmpty())
	{
		FMSaveGraphNode GraphNode;
		DepthQueue.Dequeue(GraphNode);
		ColumnStack.Add(GraphNode);
		SaveGraphNodes.Add(GraphNode.SaveId, GraphNode);
		MaximumHeight = FMath::Max(MaximumHeight, GraphNode.Position.Y + NodeSize.Y + Spacing.Y);

		for (const FGuid& ChildId : GraphNode.BranchChildren)
		{
			TArray<FGuid> NodeBranchChildren = BranchChildren.FindRef(ChildId);
			TArray<FGuid> NodeSequenceChildren = SequenceChildren.FindRef(ChildId);

			DepthQueue.Enqueue({
				ChildId,
				ChildId.ToString().Left(8),
				{ GraphNode.Position.X, GraphNode.Position.Y + NodeSize.Y + Spacing.Y },
				ChildId == SaveGame->MostRecentNodeId,
				SaveGame->SaveNodes[ChildId].bInvisible,
				NodeBranchChildren,
				NodeSequenceChildren
			 });
		}
	}

	// 3. Calculate the x position of all nodes
	int32 NextFreeColumn = 0;
	// Iterate backwards to assign columns in the correct order
	for (int32 Index = ColumnStack.Num() - 1; Index >= 0; --Index)
	{
		FMSaveGraphNode& GraphNode = SaveGraphNodes[ColumnStack[Index].SaveId];
		// if leaf node...
		if (GraphNode.BranchChildren.IsEmpty())
		{
			// ...find the first free column
			GraphNode.Position.X = NextFreeColumn++ * (NodeSize.X + Spacing.X) + Spacing.X;
		}
		else // else must be a parent node...
		{
			// ...assign the same column as its first branch child
			GraphNode.Position.X = SaveGraphNodes[GraphNode.BranchChildren[0]].Position.X;
		}

		MaximumWidth = FMath::Max(MaximumWidth, GraphNode.Position.X + NodeSize.X + Spacing.X);
	}
}

int32 SMSaveGraph::OnPaint(
	const FPaintArgs&		 Args,
	const FGeometry&		 AllottedGeometry,
	const FSlateRect&		 MyClippingRect,
	FSlateWindowElementList& OutDrawElements,
	int32					 LayerId,
	const FWidgetStyle&		 InWidgetStyle,
	bool					 bParentEnabled) const
{
	// Draw save slot label
	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(FVector2f(8.0f, 8.0f), FSlateLayoutTransform(FVector2f(0.0f, 0.0f))),
		SaveSlotLabel,
		FCoreStyle::GetDefaultFontStyle("Regular", 10),
		ESlateDrawEffect::None,
		FLinearColor::White);

	for (const TTuple<FGuid, FMSaveGraphNode>& GraphNode : SaveGraphNodes)
	{
		FVector2f Position = GraphNode.Value.Position;
		FVector2f LineStart = Position + NodeSize;

		// Draw solid lines for BranchParents
		for (const FGuid& ChildId : GraphNode.Value.BranchChildren)
		{
			FVector2f LineEnd = SaveGraphNodes[ChildId].Position;
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				{ LineStart, LineEnd },
				ESlateDrawEffect::None,
				FLinearColor::Yellow,
				true,
				2.0f);
		}

		// Draw dashed lines for SequenceParents
		for (const FGuid& ChildId : GraphNode.Value.SequenceChildren)
		{
			if (GraphNode.Value.BranchChildren.Contains(ChildId)) continue;

			FVector2f LineEnd = SaveGraphNodes[ChildId].Position;
			FSlateDrawElement::MakeDashedLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				{ LineStart, LineEnd },
				ESlateDrawEffect::None,
				FLinearColor::Yellow,
				2.0f,
				2.0f);
		}

		if (GraphNode.Value.bInvisible)
		{
			// Draw bordered boxes for invisible nodes
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				{ Position,
				  FVector2f(Position.X + NodeSize.X, Position.Y),
				  Position + NodeSize,
				  FVector2f(Position.X, Position.Y + NodeSize.Y),
				  Position },
				ESlateDrawEffect::None,
				FLinearColor(0.2f, 0.2f, 0.8f, 1.0f),
				true,
				1.5f);
		}
		else
		{
			// Draw filled boxes for visible nodes
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(NodeSize), FSlateLayoutTransform(FVector2f(Position))),
				FCoreStyle::Get().GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				FLinearColor(0.2f, 0.2f, 0.8f, 1.0f));
		}

		// Draw highlighted border around selected node
		if (GraphNode.Value.bSelected)
		{
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				{ Position - FVector2f(8.0f, 8.0f),
				  FVector2f(Position.X + NodeSize.X + 8.0f, Position.Y - 8.0f),
				  Position + NodeSize + FVector2f(8.0f, 8.0f),
				  FVector2f(Position.X - 8.0f, Position.Y + NodeSize.Y + 8.0f),
				  Position - FVector2f(8.0f, 8.0f) },
				ESlateDrawEffect::None,
				FLinearColor::Green,
				true,
				1.5f);
		}

		// Draw node label
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(
				FVector2f(NodeSize.X + 8.0f, NodeSize.Y + 8.0f), FSlateLayoutTransform(FVector2f(Position))),
			GraphNode.Value.Label,
			FCoreStyle::GetDefaultFontStyle("Regular", 10),
			ESlateDrawEffect::None,
			FLinearColor::White);
	}

	return LayerId + 2;
}

FVector2D SMSaveGraph::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(FMath::Max(MaximumWidth, NodeSize.X), FMath::Max(MaximumHeight, NodeSize.Y));
}
