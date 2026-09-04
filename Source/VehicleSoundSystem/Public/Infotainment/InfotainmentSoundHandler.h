#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataAssets/InfotainmentSoundDataAsset.h"
#include "InfotainmentSoundHandler.generated.h"

/**
 * Handles infotainment UI sound playback (touch feedback, notifications).
 * All sounds are played as 2D (non-spatialized) since they come from the in-vehicle UI.
 */
UCLASS(BlueprintType)
class VEHICLESOUNDSYSTEM_API UInfotainmentSoundHandler : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UInfotainmentSoundDataAsset* InDataAsset);

	/** Play the touch/press feedback sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Infotainment")
	void PlayTouchFeedback();

	/** Play the navigation/focus change sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Infotainment")
	void PlayNavigationSound();

	/** Play a notification sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Infotainment")
	void PlayNotification();

	/** Play the error sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Infotainment")
	void PlayErrorSound();

	void SetVolumeMultiplier(float InVolume) { VolumeMultiplier = FMath::Clamp(InVolume, 0.0f, 1.0f); }

private:
	void PlaySound2D(USoundBase* Sound);

	UPROPERTY()
	TObjectPtr<UInfotainmentSoundDataAsset> SoundData;

	float VolumeMultiplier = 1.0f;
};
