using UnrealBuildTool;
 
public class ShowSystemEditor : ModuleRules
{
	public ShowSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"UnrealEd",
                "StructUtils",                
            }
			);

		PrivateDependencyModuleNames.AddRange(
			new string[] { 
				"ShowSystem",
                "AssetTools",
                "PropertyEditor",
                "Slate",
                "SlateCore",
                "InputCore",
                "SlateEditorUtils",
                "EditorStyle",
                "ToolMenus",
                "AdvancedPreviewScene",

                "KismetWidgets",
                "EditorWidgets",                
                "AnimGraph",                
            }
			);
	}
}