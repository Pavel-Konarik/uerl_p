// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CybertoothMLEditor : ModuleRules
{
	public CybertoothMLEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",

				// ... add other public dependencies that you statically link with here ...
			}
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"AssetTools",
				"CybertoothML",
                "UnrealEd",
                "SlateCore",

                "RHI",
                "ToolMenus",
                "DeveloperSettings",
                "ClassViewer",
                "EngineSettings",
                "AssetRegistry",
				"EditorFramework",
				// ... add private dependencies that you statically link with here ...	

				"ImageWrapper",
				"MaterialBaking",
				"Projects",
				"ImageWriteQueue"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
