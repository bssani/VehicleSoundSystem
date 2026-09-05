#include "DynamicSound/EVMotorSoundLayer.h"
#include "VehicleSoundSystemModule.h"

void UEVMotorSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	Super::Initialize(InOwner, InDataAsset);

	if (DataAsset)
	{
		AudioComponent = CreateAudioComponent(DataAsset->EngineConfig.MetaSoundSource, DataAsset->EngineConfig.AttenuationOverride, DataAsset->EngineConfig.ConcurrencyOverride);
	}
}

void UEVMotorSoundLayer::Update(float DeltaTime, const FVehicleSoundState& State)
{
	if (!AudioComponent || !bIsActive)
	{
		return;
	}

	SetMetaSoundParameter(FName("RPM"), State.RPM);
	SetMetaSoundParameter(FName("Speed"), State.Speed);
	// EV torque approximated from throttle input and engine load
	float Torque = State.ThrottleInput * State.EngineLoad;
	SetMetaSoundParameter(FName("Torque"), Torque);

	if (DataAsset && DataAsset->EngineConfig.RPMToVolumeCurve)
	{
		float CurveVolume = DataAsset->EngineConfig.RPMToVolumeCurve->GetFloatValue(State.RPM);
		AudioComponent->SetVolumeMultiplier(Volume * CurveVolume);
	}
}
