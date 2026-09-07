#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VehicleSoundSettings.generated.h"

/**
 * Project-wide defaults for the vehicle sound system, shown under Project Settings > Plugins >
 * Vehicle Sound System and saved to DefaultGame.ini.
 *
 * These used to live on the subsystem. EditAnywhere on a subsystem compiles but shows up nowhere,
 * since a GameInstanceSubsystem has no editor of its own, so the distances were effectively
 * compiled in and every change needed a programmer. The subsystem still holds its own copies and
 * still lets Blueprints move them per session; it just seeds them from here on startup.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Vehicle Sound System"))
class VEHICLESOUNDSYSTEM_API UVehicleSoundSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UVehicleSoundSettings();

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	/** Distance beyond which the quieter layers stop. Wind, exhaust and transmission detail is
	 *  cabin detail and doesn't carry across a track */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Distance LOD", meta = (Units = "cm", ClampMin = "0.0"))
	float ReducedDistance = 2500.0f;

	/** Distance beyond which only the engine runs */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Distance LOD", meta = (Units = "cm", ClampMin = "0.0"))
	float EngineOnlyDistance = 6000.0f;

	/** Distance beyond which a vehicle is silent. Keep this at or under the falloff distance of
	 *  the attenuation the layers use, or vehicles are culled while still audible */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Distance LOD", meta = (Units = "cm", ClampMin = "0.0"))
	float CullDistance = 15000.0f;

	/** Volume every vehicle starts at */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	/** Engine, tyres, wind and the rest of the continuous layers */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DynamicVolume = 1.0f;

	/** Collisions, doors, indicators */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InteractionVolume = 1.0f;

	/** Radio and chimes */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InfotainmentVolume = 1.0f;
};
