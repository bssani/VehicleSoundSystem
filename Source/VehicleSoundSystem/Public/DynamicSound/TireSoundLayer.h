#pragma once

#include "CoreMinimal.h"
#include "DynamicSound/DynamicSoundLayer.h"
#include "TireSoundLayer.generated.h"

/**
 * Tire/road noise layer.
 * Drives MetaSound parameters: Speed, SurfaceType, SlipAngle
 */
UCLASS()
class VEHICLESOUNDSYSTEM_API UTireSoundLayer : public UDynamicSoundLayer
{
	GENERATED_BODY()

public:
	virtual void Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset) override;
	virtual void Update(float DeltaTime, const FVehicleSoundState& State) override;
	virtual EDynamicSoundLayerType GetLayerType() const override { return EDynamicSoundLayerType::Tire; }

private:
	/** True when no MetaSound graph was assigned and the layer is switching raw surface samples */
	bool bUsingSurfaceSamples = false;

	/** Surface the current sample was chosen for, so it only swaps when the ground changes */
	ETireSurfaceType LastSurface = ETireSurfaceType::Asphalt;

	/** What the graph was last told to play, so it is only told again when the answer changes.
	 *  Starts deliberately wrong so the first update always sends one */
	bool bWasSlipping = false;
	bool bSentFirstUpdate = false;

	/** Set when the choice of sound changed, fired on the following frame. A trigger and a float
	 *  set in the same frame are not guaranteed to reach the graph in the order they were written,
	 *  so telling it to re-pick in the same breath as changing what it should pick can have it
	 *  re-pick from the old values and stay there */
	bool bUpdatePending = false;
};
