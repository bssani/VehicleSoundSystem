#pragma once

#include "CoreMinimal.h"
#include "DynamicSound/DynamicSoundLayer.h"
#include "TransmissionSoundLayer.generated.h"

/**
 * Transmission/gearbox sound layer.
 * Drives MetaSound parameters: RPM, Gear, EngineLoad
 * Also plays one-shot gear shift sounds.
 */
UCLASS()
class VEHICLESOUNDSYSTEM_API UTransmissionSoundLayer : public UDynamicSoundLayer
{
	GENERATED_BODY()

public:
	virtual void Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset) override;
	virtual void Update(float DeltaTime, const FVehicleSoundState& State) override;
	virtual EDynamicSoundLayerType GetLayerType() const override { return EDynamicSoundLayerType::Transmission; }

private:
	/** Plays a one-shot gear shift sound */
	void PlayGearShiftSound();

	int32 LastGear = 0;
};
