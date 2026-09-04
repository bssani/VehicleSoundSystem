#pragma once

#include "CoreMinimal.h"
#include "DynamicSound/DynamicSoundLayer.h"
#include "EngineSoundLayer.generated.h"

/**
 * ICE engine sound layer.
 * Drives MetaSound parameters: RPM, EngineLoad, Throttle
 */
UCLASS()
class VEHICLESOUNDSYSTEM_API UEngineSoundLayer : public UDynamicSoundLayer
{
	GENERATED_BODY()

public:
	virtual void Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset) override;
	virtual void Update(float DeltaTime, const FVehicleSoundState& State) override;
	virtual EDynamicSoundLayerType GetLayerType() const override { return EDynamicSoundLayerType::Engine; }
};
