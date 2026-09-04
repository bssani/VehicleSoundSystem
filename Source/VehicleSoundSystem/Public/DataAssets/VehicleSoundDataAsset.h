#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundConcurrency.h"
#include "Enums/VehicleSoundEnums.h"
#include "VehicleSoundDataAsset.generated.h"

/**
 * Base data asset for vehicle sound configuration.
 * Provides common audio settings shared across all sound categories.
 */
UCLASS(Abstract, BlueprintType)
class VEHICLESOUNDSYSTEM_API UVehicleSoundDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FName AssetName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	EVehicleSoundCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
	TObjectPtr<USoundClass> SoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
	TObjectPtr<USoundAttenuation> Attenuation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
	TObjectPtr<USoundConcurrency> Concurrency;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(GetClass()->GetFName(), GetFName());
	}
};
