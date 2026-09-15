using UnrealBuildTool;
using System.Collections.Generic;

public class AegisCombatTarget : TargetRules
{
	public AegisCombatTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("AegisCombat");
	}
}
