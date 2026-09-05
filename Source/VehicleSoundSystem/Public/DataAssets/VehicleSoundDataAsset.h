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

	/** Renames the parameters this asset's layers send, so a MetaSound written for some other
	 *  system can be used without touching code. Key is the name the layer sends, value is the
	 *  name the graph listens on - for example Slip -> OnSlip, SurfaceType -> Surface.
	 *  Anything not listed is sent under its own name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSound")
	TMap<FName, FName> ParameterNameOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
	TObjectPtr<USoundConcurrency> Concurrency;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(GetClass()->GetFName(), GetFName());
	}
};
