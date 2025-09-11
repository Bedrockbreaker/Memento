// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;

public class MementoDevelopment : ModuleRules
{
	public MementoDevelopment(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine"
		});
	}
}
