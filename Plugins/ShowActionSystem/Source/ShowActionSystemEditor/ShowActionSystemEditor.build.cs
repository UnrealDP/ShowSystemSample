using UnrealBuildTool;
 
public class ShowActionSystemEditor : ModuleRules
{
	public ShowActionSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd"});
 
		PublicIncludePaths.AddRange(new string[] {"ShowActionSystemEditor/Public"});
		PrivateIncludePaths.AddRange(new string[] {"ShowActionSystemEditor/Private"});
	}
}