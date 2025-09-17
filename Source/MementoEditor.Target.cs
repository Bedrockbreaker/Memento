// Copyright © Bedrockbreaker 2025. MIT License

using UnrealBuildTool;
using System.Collections.Generic;

public class MementoEditorTarget : TargetRules
{
	public MementoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("Memento");
	}
}
