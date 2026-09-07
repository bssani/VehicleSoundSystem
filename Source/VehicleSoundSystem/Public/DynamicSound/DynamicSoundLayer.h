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

	/** The component doing the playing, for callers that need to ask what it is actually doing */
	const UAudioComponent* GetAudioComponent() const { return AudioComponent; }

	/** Restarts the source if it stopped by itself. A layer is meant to run for as long as the
	 *  vehicle does, but a source can end on its own - a MetaSound carrying the one-shot interface
	 *  ends whenever its graph says so - and nothing tells the layer. The layer is then marked
	 *  active while silent, which is the one state neither Activate nor the distance LOD can undo:
	 *  both skip a layer that already reports itself active. Without this, one such ending silences
	 *  that layer for the rest of the session */
	void RestartIfStopped();

	/** Get the layer type */
	virtual EDynamicSoundLayerType GetLayerType() const PURE_VIRTUAL(UDynamicSoundLayer::GetLayerType, return EDynamicSoundLayerType::Engine;);

	virtual void BeginDestroy() override;

protected:
	/** Creates and configures the AudioComponent with the given SoundBase */
	UAudioComponent* CreateAudioComponent(USoundBase* Sound, USoundAttenuation* AttenuationOverride = nullptr, USoundConcurrency* ConcurrencyOverride = nullptr);

	/** Maps a parameter name through the data asset's overrides. Layers always send their own
	 *  canonical names; this is what lets a third-party graph listen on different ones */
	FName ResolveParameterName(FName ParameterName) const;

	/** Helper to set a float parameter on the AudioComponent (MetaSound input) */
	void SetMetaSoundParameter(FName ParameterName, float Value);

	/** Helper to set an int parameter on the AudioComponent (MetaSound input) */
	void SetMetaSoundIntParameter(FName ParameterName, int32 Value);

	/** Helper to set a bool parameter on the AudioComponent (MetaSound input) */
	void SetMetaSoundBoolParameter(FName ParameterName, bool bValue);

	/** Fires a trigger input on the graph. Graphs that switch between sounds usually want telling
	 *  when the choice has changed rather than watching for it themselves */
	void SetMetaSoundTrigger(FName ParameterName);

	/** Sets playback pitch directly on the AudioComponent.
	 *  A MetaSound graph does its own pitch work, but a layer pointed at a plain SoundWave has
	 *  no graph to do it, and without this the sound never changes with the vehicle. Driving it
	 *  from the data asset's curves makes a single loop usable on its own. */
	void SetPlaybackPitch(float Pitch);

	/** Swaps the playing sound, resuming playback if it was already going. Used where a layer
	 *  picks between per-surface samples with no MetaSound graph to crossfade them. */
	void SwapSound(USoundBase* NewSound);

	UPROPERTY()
	TObjectPtr<UVehicleSoundComponent> OwningComponent;

	UPROPERTY()
	TObjectPtr<UDynamicSoundDataAsset> DataAsset;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent;

	/** True when the assigned source is a MetaSound. A graph does its own pitch and mixing work,
	 *  so the code-side fallbacks have to stand down or the two fight each other */
	bool bSourceIsMetaSound = false;

	float Volume = 1.0f;
	bool bIsActive = false;

	/** So a source that keeps ending reports itself once rather than every time it restarts */
	bool bReportedSourceEnded = false;
};
