// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class house : ModuleRules
{
	public house(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
            "NavigationSystem",
            "GameplayTasks",
            "GeometryCollectionEngine",
            "ChaosSolverEngine",
            "PhysicsCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"house",
			"house/Variant_Platforming",
			"house/Variant_Combat",
			"house/Variant_Combat/AI",
			"house/Variant_SideScrolling",
			"house/Variant_SideScrolling/Gameplay",
			"house/Variant_SideScrolling/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
