using UnrealBuildTool;
 
public class ShowSystemEditor : ModuleRules
{
	public ShowSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd"});

		PrivateDependencyModuleNames.AddRange(
			new string[] { 
				"ShowSystem",
                "AssetTools",
			}
			);

        PublicIncludePaths.AddRange(new string[] {"ShowSystemEditor/Public"});
		PrivateIncludePaths.AddRange(new string[] {"ShowSystemEditor/Private"});
	}
}