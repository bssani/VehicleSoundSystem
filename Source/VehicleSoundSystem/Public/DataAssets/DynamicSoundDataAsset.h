#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VehicleSoundDataAsset.h"
#include "Sound/SoundWave.h"
#include "Curves/CurveFloat.h"
#include "DynamicSoundDataAsset.generated.h"

/** Configuration for engine sound (ICE or EV) */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FEngineSoundConfig
{
	GENERATED_BODY()

	/** MetaSound source to use for engine audio (MS_EngineLoop_ICE or MS_EngineLoop_EV) */
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

	/** Curve mapping RPM to pitch multiplier. Leave unset to use the linear fallback below */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	TObjectPtr<UCurveFloat> RPMToPitchCurve;

	/** Pitch at MaxRPM when no RPMToPitchCurve is authored. Idle is always 1.0, so 2.0 means the
	 *  loop plays an octave up at redline. Only used as a fallback */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine", meta = (ClampMin = "1.0", ClampMax = "4.0"))
	float PitchAtMaxRPM = 2.0f;

	/** Optional multi-sample layers used inside MetaSound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	TArray<TObjectPtr<USoundWave>> EngineSamples;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exhaust")
	TArray<TObjectPtr<USoundWave>> ExhaustSamples;
};

/** Configuration for tire/road noise */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FTireSoundConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	TObjectPtr<USoundBase> MetaSoundSource;

	/** Sound wave per surface type for crossfade in MetaSound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	TMap<ETireSurfaceType, TObjectPtr<USoundWave>> SurfaceSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	TObjectPtr<UCurveFloat> SpeedToTireVolume;
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
};

/** Configuration for transmission/gearbox sounds */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FTransmissionSoundConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	TObjectPtr<USoundBase> MetaSoundSource;

	/** One-shot sounds for gear shift events */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	TArray<TObjectPtr<USoundWave>> GearShiftSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transmission")
	TObjectPtr<UCurveFloat> RPMToWhineVolume;
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
