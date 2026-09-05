#include "DynamicSound/TireSoundLayer.h"
#include "VehicleSoundSystemModule.h"

void UTireSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	Super::Initialize(InOwner, InDataAsset);

	if (!DataAsset)
	{
		return;
	}

	// prefer the MetaSound graph. Without one, fall back to the per-surface samples so the layer
	// still makes noise: the graph would normally crossfade these, we just switch between them
	USoundBase* Source = DataAsset->TireConfig.MetaSoundSource;

	if (!Source)
	{
		if (TObjectPtr<USoundBase>* Fallback = DataAsset->TireConfig.SurfaceSounds.Find(ETireSurfaceType::Asphalt))
		{
			Source = *Fallback;
		}
	}

	AudioComponent = CreateAudioComponent(Source, DataAsset->TireConfig.AttenuationOverride);
	bUsingSurfaceSamples = DataAsset->TireConfig.MetaSoundSource == nullptr;
}

void UTireSoundLayer::Update(float DeltaTime, const FVehicleSoundState& State)
{
	if (!AudioComponent || !bIsActive)
	{
		return;
	}

	if (!DataAsset)
	{
		return;
	}

	const FTireSoundConfig& Config = DataAsset->TireConfig;

	// with no graph to crossfade them, swap the sample when the surface underfoot changes
	if (bUsingSurfaceSamples && State.TireSurface != LastSurface)
	{
		if (const TObjectPtr<USoundBase>* SurfaceSound = Config.SurfaceSounds.Find(State.TireSurface))
		{
			SwapSound(*SurfaceSound);
		}

		LastSurface = State.TireSurface;
	}

	SetMetaSoundParameter(FName("Speed"), State.Speed);
	SetMetaSoundIntParameter(FName("SurfaceType"), static_cast<int32>(State.TireSurface));
	SetMetaSoundParameter(FName("Slip"), State.TireSlip);
	SetMetaSoundParameter(FName("Skidding"), State.bTireSkidding ? 1.0f : 0.0f);

	// rolling noise scales with speed, but sliding is what makes tyres loud. Taking the louder of
	// the two means a stationary burnout is still heard, which the old speed-only path missed
	float RollingVolume = 1.0f;

	if (Config.SpeedToTireVolume)
	{
		RollingVolume = Config.SpeedToTireVolume->GetFloatValue(State.Speed);
	}

	AudioComponent->SetVolumeMultiplier(Volume * FMath::Max(RollingVolume, State.TireSlip));
}
