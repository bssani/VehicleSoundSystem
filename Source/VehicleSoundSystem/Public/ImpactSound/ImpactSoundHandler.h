#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataAssets/DynamicSoundDataAsset.h"
#include "ImpactSoundHandler.generated.h"

class UAudioComponent;

/**
 * Plays collision sounds for a vehicle.
 *
 * Impacts are one-shots at the contact point rather than a continuous layer, so this is a
 * handler rather than a UDynamicSoundLayer. Two things it deliberately does:
 *
 * - Scales with the speed the surfaces met at, not the vehicle's speed. Sliding along a wall at
 *   100km/h is a scrape; hitting it head-on at 30 is a bang.
 * - Rate limits. Chaos reports contact every frame while two bodies stay touching, and playing a
 *   one-shot per frame turns a scrape into a machine gun.
 */
UCLASS(BlueprintType)
class VEHICLESOUNDSYSTEM_API UImpactSoundHandler : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(AActor* InOwner, UDynamicSoundDataAsset* InDataAsset);

	void Shutdown();

	/** Plays an impact of the given strength in cm/s at a world location. Exposed so a vehicle
	 *  that resolves its own collisions can report them instead of relying on hit events */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Impact")
	void ReportImpact(const FVector& Location, float ImpactSpeed);

	void SetVolumeMultiplier(float InVolume) { VolumeMultiplier = FMath::Clamp(InVolume, 0.0f, 1.0f); }

private:
	UFUNCTION()
	void HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	TObjectPtr<AActor> OwningActor;

	UPROPERTY()
	TObjectPtr<UDynamicSoundDataAsset> SoundData;

	float VolumeMultiplier = 1.0f;

	/** Time of the last impact, used to rate limit continuous contact */
	double LastImpactTime = 0.0;
};
