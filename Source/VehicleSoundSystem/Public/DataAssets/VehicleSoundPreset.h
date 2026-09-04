#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAssets/DynamicSoundDataAsset.h"
#include "DataAssets/InteractionSoundDataAsset.h"
#include "DataAssets/InfotainmentSoundDataAsset.h"
#include "VehicleSoundPreset.generated.h"

/**
 * Aggregates all three sound data assets into a single vehicle sound profile.
 * Assign this preset to VehicleSoundComponent for one-stop configuration.
 */
UCLASS(BlueprintType)
class VEHICLESOUNDSYSTEM_API UVehicleSoundPreset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
	FName PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Data")
	TObjectPtr<UDynamicSoundDataAsset> DynamicSoundData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Data")
	TObjectPtr<UInteractionSoundDataAsset> InteractionSoundData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Data")
	TObjectPtr<UInfotainmentSoundDataAsset> InfotainmentSoundData;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("VehicleSoundPreset"), GetFName());
	}
};
