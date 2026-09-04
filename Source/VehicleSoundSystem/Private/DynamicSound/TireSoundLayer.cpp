#include "DynamicSound/TireSoundLayer.h"
#include "VehicleSoundSystemModule.h"

void UTireSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	Super::Initialize(InOwner, InDataAsset);

	if (DataAsset)
	{
		AudioComponent = CreateAudioComponent(DataAsset->TireConfig.MetaSoundSource);
	}
}

void UTireSoundLayer::Update(float DeltaTime, const FVehicleSoundState& State)
{
	if (!AudioComponent || !bIsActive)
	{
		return;
	}

	SetMetaSoundParameter(FName("Speed"), State.Speed);
	SetMetaSoundIntParameter(FName("SurfaceType"), static_cast<int32>(State.TireSurface));
	// SlipAngle approximated from steering input and speed
	float SlipAngle = FMath::Abs(State.SteeringInput) * FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, 100.0f), FVector2D(0.0f, 1.0f), State.Speed);
	SetMetaSoundParameter(FName("SlipAngle"), SlipAngle);

	if (DataAsset && DataAsset->TireConfig.SpeedToTireVolume)
	{
		float CurveVolume = DataAsset->TireConfig.SpeedToTireVolume->GetFloatValue(State.Speed);
		AudioComponent->SetVolumeMultiplier(Volume * CurveVolume);
	}
}
