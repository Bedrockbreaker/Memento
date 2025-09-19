// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;

public class MementoSaveSystem : ModuleRules
{
	public MementoSaveSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"EngineSettings",
			"InputCore",
			"Slate"
		});
	}
}
