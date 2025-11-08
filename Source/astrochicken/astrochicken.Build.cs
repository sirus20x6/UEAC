// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/astrochicken.Build.cs

using UnrealBuildTool;

public class astrochicken : ModuleRules
{
	public astrochicken(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Json",
			"JsonUtilities"
		});

		// libgit2 for save system
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicAdditionalLibraries.Add("git2");
			PublicSystemIncludePaths.Add("/usr/include");
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// TODO: Add Windows libgit2 paths when needed
		}

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
