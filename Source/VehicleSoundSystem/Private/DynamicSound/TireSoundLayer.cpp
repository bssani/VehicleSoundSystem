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

	AudioComponent = CreateAudioComponent(Source, DataAsset->TireConfig.AttenuationOverride, DataAsset->TireConfig.ConcurrencyOverride);
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

	// Rolling noise scales with speed, sliding is what makes tyres loud, and the louder of the two
	// wins so that a stationary burnout is still heard.
	//
	// Rolling used to fall back to 1.0 when no curve was authored, which is the case in every
	// project that has not sat down and drawn one. That pinned the layer at full volume forever:
	// the slide could never be louder than the rolling it was supposed to rise above, so tyre slip
	// changed nothing at all and the tyres simply roared from the moment play began.
	float RollingVolume;

	if (Config.SpeedToTireVolume)
	{
		RollingVolume = Config.SpeedToTireVolume->GetFloatValue(State.Speed);
	}
	else
	{
		RollingVolume = FMath::Clamp(State.Speed / FMath::Max(Config.SpeedAtFullRollingVolume, 1.0f), 0.0f, 1.0f)
			* Config.RollingVolumeScale;
	}

	AudioComponent->SetVolumeMultiplier(Volume * FMath::Max(RollingVolume, State.TireSlip));
}
