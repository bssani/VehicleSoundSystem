#include "DynamicSound/ExhaustSoundLayer.h"
#include "VehicleSoundSystemModule.h"

void UExhaustSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	Super::Initialize(InOwner, InDataAsset);

	if (DataAsset)
	{
		AudioComponent = CreateAudioComponent(DataAsset->ExhaustConfig.MetaSoundSource);
	}
}

void UExhaustSoundLayer::Update(float DeltaTime, const FVehicleSoundState& State)
{
	if (!AudioComponent || !bIsActive)
	{
		return;
	}

	SetMetaSoundParameter(FName("RPM"), State.RPM);
	SetMetaSoundParameter(FName("EngineLoad"), State.EngineLoad);
	// Exhaust valve open is approximated from throttle - more throttle = more open
	float ExhaustValveOpen = FMath::Clamp(State.ThrottleInput * 0.8f + State.EngineLoad * 0.2f, 0.0f, 1.0f);
	SetMetaSoundParameter(FName("ExhaustValveOpen"), ExhaustValveOpen);

	if (DataAsset && DataAsset->ExhaustConfig.RPMToExhaustVolume)
	{
		float CurveVolume = DataAsset->ExhaustConfig.RPMToExhaustVolume->GetFloatValue(State.RPM);
		AudioComponent->SetVolumeMultiplier(Volume * CurveVolume);
	}
}
