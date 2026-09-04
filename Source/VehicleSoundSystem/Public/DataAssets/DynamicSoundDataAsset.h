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

	/** Curve mapping RPM to pitch multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	TObjectPtr<UCurveFloat> RPMToPitchCurve;

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
};
