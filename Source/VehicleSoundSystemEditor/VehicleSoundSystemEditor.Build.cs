using UnrealBuildTool;

public class VehicleSoundSystemEditor : ModuleRules
{
	public VehicleSoundSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"VehicleSoundSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"AssetTools",
			"PropertyEditor"
		});
	}
}
