#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Enums/VehicleSoundEnums.h"
#include "VehicleSoundSubsystem.generated.h"

class UVehicleSoundComponent;

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

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UVehicleSoundComponent>> ActiveVehicles;

	float MasterVolume = 1.0f;

	UPROPERTY()
	TMap<EVehicleSoundCategory, float> CategoryVolumes;
};
