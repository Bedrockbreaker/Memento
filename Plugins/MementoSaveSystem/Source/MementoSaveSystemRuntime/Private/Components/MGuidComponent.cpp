// Copyright © Bedrockbreaker 2025. MIT License

#include "Components/MGuidComponent.h"

UMGuidComponent::UMGuidComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
void UMGuidComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (Guid.IsValid()) return;
	Guid = FGuid::NewGuid();
	Modify();
}

void UMGuidComponent::PostLoad()
{
	Super::PostLoad();

	if (Guid.IsValid()) return;
	Guid = FGuid::NewGuid();
	Modify();
}

void UMGuidComponent::PostEditImport()
{
	Super::PostEditImport();

	// Always create a new guid when the component is duplicated
	Guid = FGuid::NewGuid();
	Modify();
}
#endif
