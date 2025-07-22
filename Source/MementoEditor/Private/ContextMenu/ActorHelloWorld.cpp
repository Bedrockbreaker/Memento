// Copyright © Bedrockbreaker 2025. MIT License

#include "ActorHelloWorld.h"

#include "EditorUtilityLibrary.h"

void UMemActorHelloWorld::PrintActorNames() const
{
	TArray<AActor*> actors = UEditorUtilityLibrary::GetSelectionSet();
	for (AActor* actor : actors)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor Name: %s"), *actor->GetName());
	}
}
