#pragma once

#include "CoreMinimal.h"
#include "DynamicSound/DynamicSoundLayer.h"
#include "EVMotorSoundLayer.generated.h"

/**
 * Electric vehicle motor sound layer.
 * Drives MetaSound parameters: RPM, Speed, Torque
 */
UCLASS()
class VEHICLESOUNDSYSTEM_API UEVMotorSoundLayer : public UDynamicSoundLayer
{
	GENERATED_BODY()

public:
	virtual void Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset) override;
	virtual void Update(float DeltaTime, const FVehicleSoundState& State) override;
	virtual EDynamicSoundLayerType GetLayerType() const override { return EDynamicSoundLayerType::EVMotor; }
};
