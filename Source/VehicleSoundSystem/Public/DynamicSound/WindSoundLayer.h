#pragma once

#include "CoreMinimal.h"
#include "DynamicSound/DynamicSoundLayer.h"
#include "WindSoundLayer.generated.h"

/**
 * Aerodynamic wind noise layer.
 * Drives MetaSound parameters: Speed, WindowOpenAmount
 */
UCLASS()
class VEHICLESOUNDSYSTEM_API UWindSoundLayer : public UDynamicSoundLayer
{
	GENERATED_BODY()

public:
	virtual void Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset) override;
	virtual void Update(float DeltaTime, const FVehicleSoundState& State) override;
	virtual EDynamicSoundLayerType GetLayerType() const override { return EDynamicSoundLayerType::Wind; }
};
