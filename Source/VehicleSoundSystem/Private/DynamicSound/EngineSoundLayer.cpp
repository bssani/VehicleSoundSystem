#include "DynamicSound/EngineSoundLayer.h"
#include "VehicleSoundSystemModule.h"

void UEngineSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	Super::Initialize(InOwner, InDataAsset);

	if (DataAsset)
	{
		AudioComponent = CreateAudioComponent(DataAsset->EngineConfig.MetaSoundSource, DataAsset->EngineConfig.AttenuationOverride);
	}
}

void UEngineSoundLayer::Update(float DeltaTime, const FVehicleSoundState& State)
{
	if (!AudioComponent || !bIsActive)
	{
		return;
	}

	if (!DataAsset)
	{
		return;
	}

	const FEngineSoundConfig& Config = DataAsset->EngineConfig;

	// 0 at idle, 1 at the top of the rev range. A graph that works off this doesn't have to know
	// the engine's absolute rev range, so the same graph fits any vehicle
	const float RevRange = FMath::Max(Config.MaxRPM - Config.IdleRPM, 1.0f);
	const float NormalizedRPM = FMath::Clamp((State.RPM - Config.IdleRPM) / RevRange, 0.0f, 1.0f);

	SetMetaSoundParameter(FName("RPM"), State.RPM);
	SetMetaSoundParameter(FName("NormalizedRPM"), NormalizedRPM);
	// some engine graphs blend road speed in alongside revs
	SetMetaSoundParameter(FName("Speed"), State.Speed);
	SetMetaSoundParameter(FName("EngineLoad"), State.EngineLoad);
	SetMetaSoundParameter(FName("Throttle"), State.ThrottleInput);
	SetMetaSoundParameter(FName("Redline"), State.RPM >= Config.RedlineRPM ? 1.0f : 0.0f);

	if (Config.RPMToVolumeCurve)
	{
		const float CurveVolume = Config.RPMToVolumeCurve->GetFloatValue(State.RPM);
		AudioComponent->SetVolumeMultiplier(Volume * CurveVolume);
	}

	// Pitch is what makes an engine sound like an engine. A MetaSound graph does this itself from
	// the RPM we just sent, so touching the component pitch on top would shift it twice. Only the
	// plain-sample path needs us to do it.
	if (!bSourceIsMetaSound)
	{
		if (Config.RPMToPitchCurve)
		{
			SetPlaybackPitch(Config.RPMToPitchCurve->GetFloatValue(State.RPM));
		}
		else
		{
			// no curve authored: play the loop faster as revs rise, which is a usable
			// approximation for a single-sample engine
			SetPlaybackPitch(FMath::Lerp(1.0f, Config.PitchAtMaxRPM, NormalizedRPM));
		}
	}
}
