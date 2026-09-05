#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VehicleSoundDataAsset.h"
#include "Sound/SoundWave.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundConcurrency.h"
#include "DynamicSoundDataAsset.generated.h"

/** Configuration for engine sound (ICE or EV) */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FEngineSoundConfig
{
	GENERATED_BODY()

	/** What this layer plays. Two ways to fill this in:
	 *
	 *  - a MetaSound that reads the RPM/NormalizedRPM/Throttle/EngineLoad/Redline parameters and
	 *    does its own sample blending. Everything below is then unused
	 *  - a plain looping SoundWave, in which case the curves below shape it
	 *
	 *  Leaving it empty means this layer is silent. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	TObjectPtr<USoundBase> MetaSoundSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	float IdleRPM = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	float MaxRPM = 7000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	float RedlineRPM = 6500.0f;

	/** Curve mapping RPM to volume multiplier (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	TObjectPtr<UCurveFloat> RPMToVolumeCurve;

	/** Curve mapping RPM to pitch multiplier. Only for the plain-SoundWave path: a MetaSound does
	 *  its own pitch from the RPM it is sent. Leave unset to use the linear fallback below */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine", meta = (EditCondition = "MetaSoundSource == nullptr"))
	TObjectPtr<UCurveFloat> RPMToPitchCurve;

	/** Pitch at MaxRPM when no RPMToPitchCurve is authored. Idle is always 1.0, so 2.0 plays the
	 *  loop an octave up at redline. Only for the plain-SoundWave path */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine", meta = (ClampMin = "1.0", ClampMax = "4.0", EditCondition = "MetaSoundSource == nullptr && RPMToPitchCurve == nullptr"))
	float PitchAtMaxRPM = 2.0f;

	/** Somewhere to keep the RPM-layered samples a MetaSound graph blends between. Nothing in
	 *  code reads this; the graph holds its own references. Safe to leave empty */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine", AdvancedDisplay)
	TArray<TObjectPtr<USoundWave>> EngineSamples;

	/** Falloff for this layer. Different sounds carry different distances - an impact reaches
	 *  much further than tyre roll - so each layer can override the asset-wide setting.
	 *  Leave empty to use the data asset's Attenuation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	TObjectPtr<USoundAttenuation> AttenuationOverride;

	/** Voice limiting for this layer. A concurrency written for one kind of sound must not be
	 *  applied to the rest: a four-voice cap meant for backfire pops will silence engines if it
	 *  is shared. Leave empty to use the data asset's Concurrency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	TObjectPtr<USoundConcurrency> ConcurrencyOverride;


};

/** Configuration for exhaust sound (ICE only) */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FExhaustSoundConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exhaust")
	TObjectPtr<USoundBase> MetaSoundSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exhaust")
	TObjectPtr<UCurveFloat> RPMToExhaustVolume;

	/** As with EngineSamples, a place to keep what the graph blends. Not read by code */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exhaust", AdvancedDisplay)
	TArray<TObjectPtr<USoundWave>> ExhaustSamples;

	/** Falloff for this layer. Different sounds carry different distances - an impact reaches
	 *  much further than tyre roll - so each layer can override the asset-wide setting.
	 *  Leave empty to use the data asset's Attenuation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exhaust")
	TObjectPtr<USoundAttenuation> AttenuationOverride;

	/** Voice limiting for this layer. A concurrency written for one kind of sound must not be
	 *  applied to the rest: a four-voice cap meant for backfire pops will silence engines if it
	 *  is shared. Leave empty to use the data asset's Concurrency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exhaust")
	TObjectPtr<USoundConcurrency> ConcurrencyOverride;


};

/** Configuration for tire/road noise */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FTireSoundConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	TObjectPtr<USoundBase> MetaSoundSource;

	/** One loop per surface, used only when no MetaSound is assigned above: the layer switches
	 *  between them as the ground changes. A graph would crossfade instead */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire", meta = (EditCondition = "MetaSoundSource == nullptr"))
	TMap<ETireSurfaceType, TObjectPtr<USoundBase>> SurfaceSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	TObjectPtr<UCurveFloat> SpeedToTireVolume;

	/** Falloff for this layer. Different sounds carry different distances - an impact reaches
	 *  much further than tyre roll - so each layer can override the asset-wide setting.
	 *  Leave empty to use the data asset's Attenuation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	TObjectPtr<USoundAttenuation> AttenuationOverride;

	/** Voice limiting for this layer. A concurrency written for one kind of sound must not be
	 *  applied to the rest: a four-voice cap meant for backfire pops will silence engines if it
	 *  is shared. Leave empty to use the data asset's Concurrency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	TObjectPtr<USoundConcurrency> ConcurrencyOverride;


};

/** Configuration for aerodynamic wind noise */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FWindSoundConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind")
	TObjectPtr<USoundBase> MetaSoundSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind")
	TObjectPtr<UCurveFloat> SpeedToWindVolume;

	/** Speed (km/h) at which wind sound becomes audible */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind")
	float OnsetSpeedKmh = 40.0f;

	/** Falloff for this layer. Different sounds carry different distances - an impact reaches
	 *  much further than tyre roll - so each layer can override the asset-wide setting.
	 *  Leave empty to use the data asset's Attenuation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind")
	TObjectPtr<USoundAttenuation> AttenuationOverride;

	/** Voice limiting for this layer. A concurrency written for one kind of sound must not be
	 *  applied to the rest: a four-voice cap meant for backfire pops will silence engines if it
	 *  is shared. Leave empty to use the data asset's Concurrency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind")
	TObjectPtr<USoundConcurrency> ConcurrencyOverride;


};

/** Configuration for transmission/gearbox sounds */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FTransmissionSoundConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	TObjectPtr<USoundBase> MetaSoundSource;

	/** One-shot sounds for gear shift events */
	/** Played on a gear change. SoundBase rather than SoundWave so a cue or MetaSound works too */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	TArray<TObjectPtr<USoundBase>> GearShiftSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	TObjectPtr<UCurveFloat> RPMToWhineVolume;

	/** Falloff for this layer. Different sounds carry different distances - an impact reaches
	 *  much further than tyre roll - so each layer can override the asset-wide setting.
	 *  Leave empty to use the data asset's Attenuation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	TObjectPtr<USoundAttenuation> AttenuationOverride;

	/** Voice limiting for this layer. A concurrency written for one kind of sound must not be
	 *  applied to the rest: a four-voice cap meant for backfire pops will silence engines if it
	 *  is shared. Leave empty to use the data asset's Concurrency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	TObjectPtr<USoundConcurrency> ConcurrencyOverride;


};

/** Configuration for collision/impact sounds */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FImpactSoundConfig
{
	GENERATED_BODY()

	/** Impact samples ordered light to heavy. The one played is picked by how hard the hit was */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
	TArray<TObjectPtr<USoundBase>> ImpactSounds;

	/** Closing speed below which a contact is ignored. Cars rest against things and nudge kerbs
	 *  constantly; without a floor here the car is permanently clattering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact", meta = (Units = "cm/s"))
	float MinImpactSpeed = 150.0f;

	/** Closing speed treated as a full-force crash */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact", meta = (Units = "cm/s"))
	float MaxImpactSpeed = 1500.0f;

	/** Works out impacts from how sharply the vehicle's velocity changes, instead of waiting for
	 *  contact events.
	 *
	 *  Contact callbacks are the obvious source but they cannot be relied on: with Chaos async
	 *  physics (bSubsteppingAsync) the solver runs off the game thread and OnActorHit never
	 *  arrives, so a car can hit a wall in total silence. A sudden loss of speed is unambiguous
	 *  and needs nothing from the physics callback plumbing.
	 *
	 *  The cost is that there's no contact point or surface, so the sound plays at the vehicle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
	bool bDetectImpactsFromVelocity = true;

	/** Turns on hit notifications for the owner's simulating components.
	 *
	 *  Physics bodies do not report contact unless asked to, and the flag is off by default, so
	 *  without this the collision sounds never fire and nothing says why. Switch it off if the
	 *  project manages that flag itself. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
	bool bEnableHitEventsOnOwner = true;

	/** Rate limit. Chaos reports contact every frame while bodies stay touching */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact", meta = (Units = "s"))
	float MinTimeBetweenImpacts = 0.12f;

	/** Volume of the lightest impact that still plays. Full force is always 1.0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinImpactVolume = 0.25f;

	/** Pitch for a light knock. Above 1 reads as small and tinny */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.1"))
	float PitchAtLightImpact = 1.25f;

	/** Pitch for a heavy crash. Below 1 reads as large and heavy */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.1"))
	float PitchAtHeavyImpact = 0.85f;

	/** Looping sound for sliding along a surface. Hitting a wall and scraping down it are
	 *  different sounds, and one-shots alone can only ever give the first. A MetaSound here is
	 *  sent Speed and Volume */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scrape")
	TObjectPtr<USoundBase> ScrapeSound;

	/** Sliding speed below which a contact isn't scraping, just resting against something */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scrape", meta = (Units = "cm/s"))
	float MinScrapeSpeed = 100.0f;

	/** Sliding speed treated as a full-volume scrape */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scrape", meta = (Units = "cm/s"))
	float MaxScrapeSpeed = 1200.0f;

	/** How long after the last contact the scrape keeps going. Contact events are intermittent
	 *  even while genuinely sliding, so stopping the instant one is missed makes it stutter */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scrape", meta = (Units = "s"))
	float ScrapeStopDelay = 0.15f;

	/** Falloff for this layer. Different sounds carry different distances - an impact reaches
	 *  much further than tyre roll - so each layer can override the asset-wide setting.
	 *  Leave empty to use the data asset's Attenuation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scrape")
	TObjectPtr<USoundAttenuation> AttenuationOverride;

	/** Voice limiting for this layer. A concurrency written for one kind of sound must not be
	 *  applied to the rest: a four-voice cap meant for backfire pops will silence engines if it
	 *  is shared. Leave empty to use the data asset's Concurrency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scrape")
	TObjectPtr<USoundConcurrency> ConcurrencyOverride;


};

/**
 * Data asset for dynamic (driving) sounds.
 * Contains all configuration for engine, exhaust, tire, wind, and transmission layers.
 * Separate assets should be created for ICE and EV vehicles.
 */
UCLASS(BlueprintType)
class VEHICLESOUNDSYSTEM_API UDynamicSoundDataAsset : public UVehicleSoundDataAsset
{
	GENERATED_BODY()

public:
	UDynamicSoundDataAsset()
	{
		Category = EVehicleSoundCategory::Dynamic;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powertrain")
	EVehiclePowertrainType PowertrainType = EVehiclePowertrainType::ICE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine", meta = (EditCondition = "PowertrainType == EVehiclePowertrainType::ICE || PowertrainType == EVehiclePowertrainType::EV"))
	FEngineSoundConfig EngineConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exhaust", meta = (EditCondition = "PowertrainType == EVehiclePowertrainType::ICE"))
	FExhaustSoundConfig ExhaustConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	FTireSoundConfig TireConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wind")
	FWindSoundConfig WindConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	FTransmissionSoundConfig TransmissionConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
	FImpactSoundConfig ImpactConfig;
};
