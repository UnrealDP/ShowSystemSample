// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShowSystemSample : ModuleRules
{
	public ShowSystemSample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
    }
}
