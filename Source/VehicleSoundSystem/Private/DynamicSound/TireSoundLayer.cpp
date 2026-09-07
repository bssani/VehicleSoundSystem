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

	// the component has already decided this, with hysteresis, so the answer does not flicker.
	// Volume still follows the slip amount further down; this only picks which sound plays.
	const bool bSlipping = State.bTireSliding;

	SetMetaSoundParameter(FName("Speed"), State.Speed);
	SetMetaSoundIntParameter(FName("SurfaceType"), static_cast<int32>(State.TireSurface));
	SetMetaSoundParameter(FName("Slip"), Config.bSlipParameterIsSwitch ? (bSlipping ? 1.0f : 0.0f) : State.TireSlip);
	SetMetaSoundParameter(FName("Skidding"), State.bTireSkidding ? 1.0f : 0.0f);

	// A tyre graph usually carries more than one sound: rolling, sliding, running flat, running on
	// the bare rim. It picks between them from the condition of the tyres, and left unset that
	// condition reads as a car with no tyres on it at all - so a perfectly healthy car grinds
	// along on its rims for the whole session, which is easy to mistake for a stuck slip sound.
	SetMetaSoundParameter(FName("AnyWheelHasTire"), State.TireIntactFraction);
	SetMetaSoundParameter(FName("FlatTire"), State.TireFlatFraction);
	SetMetaSoundParameter(FName("NoTire"), State.TireMissingFraction);

	// and it wants telling when the choice has changed rather than watching for it itself, one
	// frame later so the values it re-picks from are the new ones
	if (bUpdatePending)
	{
		SetMetaSoundTrigger(FName("UpdateSound"));
		bUpdatePending = false;
	}

	if (!bSentFirstUpdate || bSlipping != bWasSlipping || State.TireSurface != LastSurface)
	{
		bUpdatePending = true;
		bSentFirstUpdate = true;
		bWasSlipping = bSlipping;
		LastSurface = State.TireSurface;
	}

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
