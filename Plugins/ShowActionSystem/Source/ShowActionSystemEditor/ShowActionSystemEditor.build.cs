using System.IO;
using UnrealBuildTool;
 
public class ShowActionSystemEditor : ModuleRules
{
	public ShowActionSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd"});
 
		PublicIncludePaths.AddRange(new string[] 
        {
            Path.Combine(ModuleDirectory, "Public")
        });

		PrivateIncludePaths.AddRange(new string[] 
        {
            Path.Combine(ModuleDirectory, "Private")
        });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "InputCore",
                "EditorFramework",
                "ShowActionSystem",
                "DataTableSubsystem",
                "CoreUtils",
                "PropertyEditor",
                "ShowSystem",
                "SlateEditorUtils",
                "ToolMenus",
				// ... add private dependencies that you statically link with here ...	
			}
            );
    }
}