// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;

public class MementoEditor : ModuleRules
{
	public MementoEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		if (!Target.bBuildEditor) throw new BuildException("MementoEditor can only be built as an editor module.");

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",

			"UnrealEd",
			"Blutility"
		});

		IWYUSupport = IWYUSupport.Full;
	}
}
