#include "DynamicSound/WindSoundLayer.h"
#include "VehicleSoundSystemModule.h"

void UWindSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	Super::Initialize(InOwner, InDataAsset);

	if (DataAsset)
	{
		AudioComponent = CreateAudioComponent(DataAsset->WindConfig.MetaSoundSource, DataAsset->WindConfig.AttenuationOverride);
	}
}

void UWindSoundLayer::Update(float DeltaTime, const FVehicleSoundState& State)
{
	if (!AudioComponent || !bIsActive)
	{
		return;
	}

	SetMetaSoundParameter(FName("Speed"), State.Speed);
	SetMetaSoundParameter(FName("WindowOpenAmount"), State.WindowOpenAmount);

	// Apply onset speed - fade in wind from onset speed
	float OnsetSpeed = DataAsset ? DataAsset->WindConfig.OnsetSpeedKmh : 40.0f;
	float WindFactor = FMath::GetMappedRangeValueClamped(
		FVector2D(OnsetSpeed, OnsetSpeed + 60.0f), FVector2D(0.0f, 1.0f), State.Speed);

	if (DataAsset && DataAsset->WindConfig.SpeedToWindVolume)
	{
		float CurveVolume = DataAsset->WindConfig.SpeedToWindVolume->GetFloatValue(State.Speed);
		AudioComponent->SetVolumeMultiplier(Volume * CurveVolume * WindFactor);
	}
	else
	{
		AudioComponent->SetVolumeMultiplier(Volume * WindFactor);
	}
}
