// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;

public class MementoSaveSystemDeveloper : ModuleRules
{
	public MementoSaveSystemDeveloper(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",

			"EngineSettings",
			"InputCore",
			"Slate",

			"MementoSaveSystemRuntime"
		});
	}
}
