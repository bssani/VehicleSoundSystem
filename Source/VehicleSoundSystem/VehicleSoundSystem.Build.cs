using UnrealBuildTool;

public class VehicleSoundSystem : ModuleRules
{
	public VehicleSoundSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AudioExtensions",
			"MetasoundEngine",
			"MetasoundFrontend",
			"MetasoundGraphCore",
			"ChaosVehicles",
			"PhysicsCore",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AudioModulation"
		});
	}
}
