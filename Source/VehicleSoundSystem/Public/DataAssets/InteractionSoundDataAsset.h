#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VehicleSoundDataAsset.h"
#include "InteractionSoundDataAsset.generated.h"

/**
 * Data asset for interaction sounds (doors, buttons, seatbelt, turn signals, etc.)
 * Each interaction type maps to a USoundBase for one-shot playback.
 */
UCLASS(BlueprintType)
class VEHICLESOUNDSYSTEM_API UInteractionSoundDataAsset : public UVehicleSoundDataAsset
{
	GENERATED_BODY()

public:
	UInteractionSoundDataAsset()
	{
		Category = EVehicleSoundCategory::Interaction;
	}

	/** Map of interaction type to sound asset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction Sounds")
	TMap<EInteractionSoundType, TObjectPtr<USoundBase>> InteractionSounds;
};
