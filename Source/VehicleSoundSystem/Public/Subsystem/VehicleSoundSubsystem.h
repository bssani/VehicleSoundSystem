#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Enums/VehicleSoundEnums.h"
#include "VehicleSoundSubsystem.generated.h"

class UVehicleSoundComponent;

/** How much of a vehicle's sound is worth running at its distance from the listener */
UENUM(BlueprintType)
enum class EVehicleSoundLOD : uint8
{
	/** Every layer runs */
	Full,

	/** Engine and tyres only. Wind, exhaust and transmission detail doesn't survive the distance */
	Reduced,

	/** Engine only */
	EngineOnly,

	/** Nothing runs */
	Culled
};

/**
 * Global subsystem managing all active vehicle sound components.
 * Provides master/category volume control and distance-based LOD.
 */
UCLASS()
class VEHICLESOUNDSYSTEM_API UVehicleSoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Register a vehicle sound component (called automatically from BeginPlay) */
	void RegisterVehicle(UVehicleSoundComponent* Component);

	/** Unregister a vehicle sound component (called automatically from EndPlay) */
	void UnregisterVehicle(UVehicleSoundComponent* Component);

	/** Set master volume (affects all vehicles, all categories) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Global")
	void SetMasterVolume(float InVolume);

	/** Set volume for a specific sound category across all vehicles */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Global")
	void SetCategoryVolume(EVehicleSoundCategory Category, float InVolume);

	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|Global")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|Global")
	float GetCategoryVolume(EVehicleSoundCategory Category) const;

	/** Get number of currently active vehicles */
	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|Global")
	int32 GetActiveVehicleCount() const { return ActiveVehicles.Num(); }

	/** Returns how much of a vehicle's sound to run at the given distance from the listener.
	 *  A grid of AI cars is otherwise N vehicles times six layers of always-on audio */
	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|LOD")
	EVehicleSoundLOD GetLODForDistance(float Distance) const;

	/** Distance beyond which the quieter layers stop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|LOD", meta = (Units = "cm"))
	float ReducedDistance = 2500.0f;

	/** Distance beyond which only the engine runs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|LOD", meta = (Units = "cm"))
	float EngineOnlyDistance = 6000.0f;

	/** Distance beyond which a vehicle is silent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|LOD", meta = (Units = "cm"))
	float CullDistance = 15000.0f;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UVehicleSoundComponent>> ActiveVehicles;

	float MasterVolume = 1.0f;

	UPROPERTY()
	TMap<EVehicleSoundCategory, float> CategoryVolumes;
};
