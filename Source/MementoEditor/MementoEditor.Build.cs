// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;

public class MementoEditor : ModuleRules
{
	public MementoEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"EditorScriptingUtilities"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd",
				"Blutility"
			});
		}
	}
}
