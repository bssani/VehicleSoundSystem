#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/AudioComponent.h"
#include "Enums/VehicleSoundEnums.h"
#include "DataAssets/DynamicSoundDataAsset.h"
#include "DynamicSoundLayer.generated.h"

class UVehicleSoundComponent;

/**
 * Abstract base class for dynamic sound layers.
 * Each layer wraps a UAudioComponent playing a MetaSound source
 * and updates its parameters every tick based on vehicle state.
 */
UCLASS(Abstract)
class VEHICLESOUNDSYSTEM_API UDynamicSoundLayer : public UObject
{
	GENERATED_BODY()

public:
	/** Initialize this layer, creating the AudioComponent on the owning actor */
	virtual void Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset);

	/** Called every tick with current vehicle state */
	virtual void Update(float DeltaTime, const FVehicleSoundState& State) PURE_VIRTUAL(UDynamicSoundLayer::Update, );

	/** Start playing this layer */
	virtual void Activate();

	/** Stop playing this layer */
	virtual void Deactivate();

	/** Set volume multiplier (0-1) */
	virtual void SetVolume(float InVolume);

	/** Get current volume multiplier */
	float GetVolume() const { return Volume; }

	/** Whether this layer is currently active */
	bool IsActive() const { return bIsActive; }

	/** Get the layer type */
	virtual EDynamicSoundLayerType GetLayerType() const PURE_VIRTUAL(UDynamicSoundLayer::GetLayerType, return EDynamicSoundLayerType::Engine;);

	virtual void BeginDestroy() override;

protected:
	/** Creates and configures the AudioComponent with the given SoundBase */
	UAudioComponent* CreateAudioComponent(USoundBase* Sound);

	/** Helper to set a float parameter on the AudioComponent (MetaSound input) */
	void SetMetaSoundParameter(FName ParameterName, float Value);

	/** Helper to set an int parameter on the AudioComponent (MetaSound input) */
	void SetMetaSoundIntParameter(FName ParameterName, int32 Value);

	UPROPERTY()
	TObjectPtr<UVehicleSoundComponent> OwningComponent;

	UPROPERTY()
	TObjectPtr<UDynamicSoundDataAsset> DataAsset;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent;

	float Volume = 1.0f;
	bool bIsActive = false;
};
