// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;
using System.Collections.Generic;

public class MementoTarget : TargetRules
{
	public MementoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("Memento");
	}
}
