// Copyright Axel Woermann

using UnrealBuildTool;
using System.Collections.Generic;

public class AuraEditorTarget : TargetRules
{
	public AuraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;

		ExtraModuleNames.AddRange( new string[] { "Aura" } );
	}
}
