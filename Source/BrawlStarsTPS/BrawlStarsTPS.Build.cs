// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BrawlStarsTPS : ModuleRules
{
	public BrawlStarsTPS(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"BrawlStarsTPS",
			"BrawlStarsTPS/Variant_Platforming",
			"BrawlStarsTPS/Variant_Platforming/Animation",
			"BrawlStarsTPS/Variant_Combat",
			"BrawlStarsTPS/Variant_Combat/AI",
			"BrawlStarsTPS/Variant_Combat/Animation",
			"BrawlStarsTPS/Variant_Combat/Gameplay",
			"BrawlStarsTPS/Variant_Combat/Interfaces",
			"BrawlStarsTPS/Variant_Combat/UI",
			"BrawlStarsTPS/Variant_SideScrolling",
			"BrawlStarsTPS/Variant_SideScrolling/AI",
			"BrawlStarsTPS/Variant_SideScrolling/Gameplay",
			"BrawlStarsTPS/Variant_SideScrolling/Interfaces",
			"BrawlStarsTPS/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
