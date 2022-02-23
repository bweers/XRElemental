// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ElementalController : ModuleRules
{
	public ElementalController(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara" });

		// Add new dependency for SteamVR so its header files are used in VRPlayerPawn
		PrivateDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay", "SteamVR" });

		PublicIncludePaths.AddRange(new string[] { "C:/Program Files/Epic Games/UE_4.27/Engine/Plugins/FX/Niagara/Source/Niagara/Public" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
