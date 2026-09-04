#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Enums/VehicleSoundEnums.h"
#include "DataAssets/InteractionSoundDataAsset.h"
#include "InteractionSoundHandler.generated.h"

/**
 * Handles one-shot interaction sound playback (doors, buttons, seatbelt, etc.)
 * Supports repeating sounds (e.g. turn signal tick) via timer.
 */
UCLASS(BlueprintType)
class VEHICLESOUNDSYSTEM_API UInteractionSoundHandler : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(AActor* InOwner, UInteractionSoundDataAsset* InDataAsset);

	/** Play a one-shot interaction sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Interaction")
	void PlaySound(EInteractionSoundType Type);

	/** Start a repeating sound (e.g., turn signal tick) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Interaction")
	void StartRepeatingSound(EInteractionSoundType Type, float IntervalSeconds = 0.5f);

	/** Stop a repeating sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Interaction")
	void StopRepeatingSound(EInteractionSoundType Type);

	/** Stop all repeating sounds */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Interaction")
	void StopAllRepeatingSounds();

	void SetVolumeMultiplier(float InVolume) { VolumeMultiplier = FMath::Clamp(InVolume, 0.0f, 1.0f); }

private:
	UPROPERTY()
	TObjectPtr<AActor> OwningActor;

	UPROPERTY()
	TObjectPtr<UInteractionSoundDataAsset> SoundData;

	float VolumeMultiplier = 1.0f;

	TMap<EInteractionSoundType, FTimerHandle> RepeatingTimerHandles;
};
