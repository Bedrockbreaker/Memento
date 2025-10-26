// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;

public class MementoPlayerFramework : ModuleRules
{
	public MementoPlayerFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",

			"Memento",
			"MementoSaveSystemRuntime"
		});
	}
}
