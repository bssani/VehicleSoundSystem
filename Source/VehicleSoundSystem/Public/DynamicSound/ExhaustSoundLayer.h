#pragma once

#include "CoreMinimal.h"
#include "DynamicSound/DynamicSoundLayer.h"
#include "ExhaustSoundLayer.generated.h"

/**
 * Exhaust sound layer (ICE vehicles only).
 * Drives MetaSound parameters: RPM, EngineLoad, ExhaustValveOpen
 */
UCLASS()
class VEHICLESOUNDSYSTEM_API UExhaustSoundLayer : public UDynamicSoundLayer
{
	GENERATED_BODY()

public:
	virtual void Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset) override;
	virtual void Update(float DeltaTime, const FVehicleSoundState& State) override;
	virtual EDynamicSoundLayerType GetLayerType() const override { return EDynamicSoundLayerType::Exhaust; }
};
