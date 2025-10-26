// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;

public class MementoSaveSystemEditor : ModuleRules
{
	public MementoSaveSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",

			"UnrealEd",
			"SlateCore",
			"Slate",

			"MementoSaveSystemRuntime"
		});
	}
}
