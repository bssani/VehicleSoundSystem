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
};
