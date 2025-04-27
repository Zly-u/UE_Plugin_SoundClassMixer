// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SoundClassMixer : ModuleRules
{
	public SoundClassMixer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		// PCHUsage = ModuleRules.PCHUsageMode.NoPCHs;
		// OptimizeCode = CodeOptimization.Never;
		
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
				"Engine",
				"SignalProcessing",
				"DeveloperSettings"
				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// "UnrealEd",
				"CoreUObject",
				// ... add private dependencies that you statically link with here ...	
			}
		);
	}
}
