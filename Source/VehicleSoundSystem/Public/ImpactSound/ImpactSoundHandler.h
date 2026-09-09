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

	/** Fades out the scrape once contact stops. Contact only reports while it is happening, so
	 *  something has to notice when it stops */
	void Tick(float DeltaTime);

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

	/** Loop played while sliding along something. Created on the first scrape */
	UPROPERTY()
	TObjectPtr<UAudioComponent> ScrapeAudio;

	/** Time of the last sliding contact, used to decide when the scrape has ended */
	double LastScrapeTime = 0.0;

	/** Sliding speed of the most recent contact, fed to the scrape sound */
	float CurrentScrapeSpeed = 0.0f;

	/** Velocity and position from the previous frame, used to spot impacts as a sudden loss of
	 *  speed and to tell a real collision from a teleport */
	FVector PreviousVelocity = FVector::ZeroVector;
	FVector PreviousLocation = FVector::ZeroVector;

	/** Speed lost so far in the collision being gathered, and where it started. One frame of a
	 *  collision is not the collision */
	float GatheredSpeedLost = 0.0f;
	FVector GatheredLocation = FVector::ZeroVector;
	float GatherElapsed = 0.0f;
	bool bGathering = false;
	bool bHasPreviousFrame = false;

	/** Biggest impact heard during the contact still in progress, and when contact was last
	 *  reported. Together these turn a scrape along a wall back into a single collision */
	float ContactPeakSpeed = 0.0f;
	double LastContactTime = 0.0;

	/** Looks for an impact in this frame's change of velocity */
	void DetectImpactFromVelocity(float DeltaTime);

	/** Starts or updates the scrape loop for a contact sliding at the given speed */
	void UpdateScrape(const FVector& Location, float SlidingSpeed);
};
