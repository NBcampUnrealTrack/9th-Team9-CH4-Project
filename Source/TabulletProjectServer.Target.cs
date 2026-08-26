using UnrealBuildTool;
using System.Collections.Generic;

public class TabulletProjectServerTarget : TargetRules
{
	public TabulletProjectServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("TabulletProject");
	}
}