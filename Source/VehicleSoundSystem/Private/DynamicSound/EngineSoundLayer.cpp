#include "DynamicSound/EngineSoundLayer.h"
#include "VehicleSoundSystemModule.h"

void UEngineSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	Super::Initialize(InOwner, InDataAsset);

	if (DataAsset)
	{
		AudioComponent = CreateAudioComponent(DataAsset->EngineConfig.MetaSoundSource);
	}
}

void UEngineSoundLayer::Update(float DeltaTime, const FVehicleSoundState& State)
{
	if (!AudioComponent || !bIsActive)
	{
		return;
	}

	SetMetaSoundParameter(FName("RPM"), State.RPM);
	SetMetaSoundParameter(FName("EngineLoad"), State.EngineLoad);
	SetMetaSoundParameter(FName("Throttle"), State.ThrottleInput);

	// Apply volume curve from data asset if available
	if (DataAsset && DataAsset->EngineConfig.RPMToVolumeCurve)
	{
		float CurveVolume = DataAsset->EngineConfig.RPMToVolumeCurve->GetFloatValue(State.RPM);
		AudioComponent->SetVolumeMultiplier(Volume * CurveVolume);
	}
}
