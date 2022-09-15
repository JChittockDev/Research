// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class TissueGraph : ModuleRules
{
	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
	}

	public bool SetupTetgenLibrary(ReadOnlyTargetRules Target)
	{
		string LibPath = "";
		bool isLibrarySupported = false;
		string LibBasePath = Path.Combine(ThirdPartyPath, "tetgen");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			LibPath = Path.Combine(LibBasePath, "Libraries", "Win64");
			isLibrarySupported = true;
		}
		else
		{
			System.Console.WriteLine("Unsupported platform: {0}", Target.Platform.ToString());
		}

		if (isLibrarySupported)
		{
			PublicIncludePaths.AddRange(new string[] { Path.Combine(LibBasePath, "Includes") });
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "tetgen.lib"));
		}

		return isLibrarySupported;
	}

	private void SetupThirdPartyLibraries(ReadOnlyTargetRules Target)
	{
		SetupTetgenLibrary(Target);
	}

	public TissueGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "OptimusCore", "Projects", "RenderCore", "RHI"});
		PrivateDependencyModuleNames.AddRange(new string[] {"ComputeFramework", "CoreUObject", "Engine", "OptimusCore", "Projects", "RenderCore", "RHI"});
		SetupThirdPartyLibraries(Target);
	}
}
