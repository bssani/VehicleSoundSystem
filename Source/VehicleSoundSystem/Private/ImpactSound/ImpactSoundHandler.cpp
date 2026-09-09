#include "ImpactSound/ImpactSoundHandler.h"
#include "VehicleSoundSystemModule.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"

void UImpactSoundHandler::Initialize(AActor* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	OwningActor = InOwner;
	SoundData = InDataAsset;

	if (!OwningActor)
	{
		return;
	}

	OwningActor->OnActorHit.AddDynamic(this, &UImpactSoundHandler::HandleActorHit);

	if (!SoundData || !SoundData->ImpactConfig.bEnableHitEventsOnOwner)
	{
		return;
	}

	// A physics body stays silent about its collisions unless told to report them, and the flag
	// is off by default. Leaving that to whoever sets up the vehicle means impact sounds usually
	// just don't happen, with nothing in the log to explain it.
	TInlineComponentArray<UPrimitiveComponent*> Primitives;
	OwningActor->GetComponents(Primitives);

	for (UPrimitiveComponent* Primitive : Primitives)
	{
		if (Primitive && Primitive->IsSimulatingPhysics() && !Primitive->BodyInstance.bNotifyRigidBodyCollision)
		{
			Primitive->SetNotifyRigidBodyCollision(true);

			UE_LOG(LogVehicleSoundSystem, Log,
				TEXT("ImpactSoundHandler: enabled hit notifications on '%s' so collisions can be heard."),
				*Primitive->GetName());
		}
	}
}

void UImpactSoundHandler::Shutdown()
{
	if (OwningActor)
	{
		OwningActor->OnActorHit.RemoveDynamic(this, &UImpactSoundHandler::HandleActorHit);
	}

	if (ScrapeAudio)
	{
		ScrapeAudio->Stop();
		ScrapeAudio->DestroyComponent();
		ScrapeAudio = nullptr;
	}

	OwningActor = nullptr;
}

void UImpactSoundHandler::Tick(float DeltaTime)
{
	DetectImpactFromVelocity(DeltaTime);

	if (!ScrapeAudio || !ScrapeAudio->IsPlaying() || !SoundData || !OwningActor)
	{
		return;
	}

	const UWorld* World = OwningActor->GetWorld();

	if (!World)
	{
		return;
	}

	// contact reports arrive in bursts even during a continuous slide, so the scrape only ends
	// once nothing has been reported for a while
	if (World->GetTimeSeconds() - LastScrapeTime > SoundData->ImpactConfig.ScrapeStopDelay)
	{
		ScrapeAudio->Stop();
		CurrentScrapeSpeed = 0.0f;
	}
}

void UImpactSoundHandler::HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!SelfActor)
	{
		return;
	}

	// how fast the surfaces closed on each other, not how fast the car was travelling. A car
	// sliding along a barrier has a high speed but almost no closing velocity
	const FVector RelativeVelocity = SelfActor->GetVelocity() - (OtherActor ? OtherActor->GetVelocity() : FVector::ZeroVector);
	const float ClosingSpeed = FMath::Abs(FVector::DotProduct(RelativeVelocity, Hit.ImpactNormal));

	// whatever isn't going into the surface is going along it. Hitting a wall and sliding down it
	// are the same event split between these two numbers
	const FVector SlidingVelocity = RelativeVelocity - FVector::DotProduct(RelativeVelocity, Hit.ImpactNormal) * Hit.ImpactNormal;

	ReportImpact(Hit.ImpactPoint, ClosingSpeed);
	UpdateScrape(Hit.ImpactPoint, SlidingVelocity.Size());
}

void UImpactSoundHandler::ReportImpact(const FVector& Location, float ImpactSpeed)
{
	if (!SoundData || !OwningActor)
	{
		return;
	}

	const FImpactSoundConfig& Config = SoundData->ImpactConfig;

	if (Config.ImpactSounds.Num() == 0)
	{
		return;
	}

	const UWorld* World = OwningActor->GetWorld();

	if (!World)
	{
		return;
	}

	const double Now = World->GetTimeSeconds();

	// Contact is reported every frame while two bodies stay touching, whichever route found it.
	// A gap means whatever comes next is a new collision rather than more of this one.
	if (Now - LastContactTime > Config.ContactReleaseTime)
	{
		ContactPeakSpeed = 0.0f;
	}

	LastContactTime = Now;

	// Rubbing along a barrier reports contact for as long as it lasts, and playing a one-shot each
	// time turns one collision into a rattle. Only a hit clearly bigger than the biggest so far in
	// this contact is heard again, so a scrape sounds once and a scrape that becomes a real crash
	// is still heard.
	const float Threshold = FMath::Max(Config.MinImpactSpeed, ContactPeakSpeed * Config.ImpactEscalation);

	if (ImpactSpeed < Threshold)
	{
		return;
	}

	if (Now - LastImpactTime < Config.MinTimeBetweenImpacts)
	{
		return;
	}

	LastImpactTime = Now;
	ContactPeakSpeed = ImpactSpeed;

	// 0 at the threshold where impacts start being audible, 1 at a full-speed crash
	const float SpeedRange = FMath::Max(Config.MaxImpactSpeed - Config.MinImpactSpeed, 1.0f);
	const float Severity = FMath::Clamp((ImpactSpeed - Config.MinImpactSpeed) / SpeedRange, 0.0f, 1.0f);

	// pick the sample for this severity: with several assigned, the last is the heaviest hit
	const int32 Index = FMath::Clamp(FMath::FloorToInt(Severity * Config.ImpactSounds.Num()), 0, Config.ImpactSounds.Num() - 1);
	USoundBase* Sound = Config.ImpactSounds[Index];

	if (!Sound)
	{
		return;
	}

	// a light knock should be quiet and a heavy one shouldn't just be louder, it should be
	// lower pitched, which is what makes a hit read as heavy
	const float ImpactVolume = VolumeMultiplier * FMath::Lerp(Config.MinImpactVolume, 1.0f, Severity);
	const float ImpactPitch = FMath::Lerp(Config.PitchAtLightImpact, Config.PitchAtHeavyImpact, Severity);

	UGameplayStatics::PlaySoundAtLocation(
		OwningActor,
		Sound,
		Location,
		FRotator::ZeroRotator,
		ImpactVolume,
		ImpactPitch,
		0.0f,
		Config.AttenuationOverride ? Config.AttenuationOverride.Get() : SoundData->Attenuation.Get(),
		Config.ConcurrencyOverride ? Config.ConcurrencyOverride.Get() : SoundData->Concurrency.Get());

	UE_LOG(LogVehicleSoundSystem, Verbose, TEXT("Impact at %.0f cm/s (severity %.2f) on %s"),
		ImpactSpeed, Severity, *OwningActor->GetName());
}

void UImpactSoundHandler::UpdateScrape(const FVector& Location, float SlidingSpeed)
{
	if (!SoundData || !OwningActor)
	{
		return;
	}

	const FImpactSoundConfig& Config = SoundData->ImpactConfig;

	if (!Config.ScrapeSound || SlidingSpeed < Config.MinScrapeSpeed)
	{
		return;
	}

	const UWorld* World = OwningActor->GetWorld();

	if (!World)
	{
		return;
	}

	LastScrapeTime = World->GetTimeSeconds();
	CurrentScrapeSpeed = SlidingSpeed;

	if (!ScrapeAudio)
	{
		ScrapeAudio = UGameplayStatics::SpawnSoundAttached(
			Config.ScrapeSound,
			OwningActor->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			false,
			1.0f,
			1.0f,
			0.0f,
			Config.AttenuationOverride ? Config.AttenuationOverride.Get() : SoundData->Attenuation.Get(),
			Config.ConcurrencyOverride ? Config.ConcurrencyOverride.Get() : SoundData->Concurrency.Get(),
			false);

		if (!ScrapeAudio)
		{
			return;
		}
	}

	// follows the contact point rather than the car, so a scrape down the driver's side is heard
	// on that side
	ScrapeAudio->SetWorldLocation(Location);

	const float SpeedRange = FMath::Max(Config.MaxScrapeSpeed - Config.MinScrapeSpeed, 1.0f);
	const float Intensity = FMath::Clamp((SlidingSpeed - Config.MinScrapeSpeed) / SpeedRange, 0.0f, 1.0f);

	ScrapeAudio->SetFloatParameter(FName("Speed"), SlidingSpeed);
	ScrapeAudio->SetFloatParameter(FName("Volume"), Intensity);
	ScrapeAudio->SetVolumeMultiplier(VolumeMultiplier * Intensity);

	if (!ScrapeAudio->IsPlaying())
	{
		ScrapeAudio->Play();
	}
}

void UImpactSoundHandler::DetectImpactFromVelocity(float DeltaTime)
{
	if (!SoundData || !OwningActor || !SoundData->ImpactConfig.bDetectImpactsFromVelocity || DeltaTime <= 0.0f)
	{
		return;
	}

	const FVector Velocity = OwningActor->GetVelocity();
	const FVector Location = OwningActor->GetActorLocation();

	// nothing to compare against on the first frame
	if (!bHasPreviousFrame)
	{
		PreviousVelocity = Velocity;
		PreviousLocation = Location;
		bHasPreviousFrame = true;
		return;
	}

	const float SpeedLost = (Velocity - PreviousVelocity).Size();
	const float DistanceMoved = (Location - PreviousLocation).Size();

	// a reset or respawn also zeroes velocity, which looks exactly like hitting a wall. Real
	// motion covers roughly speed times delta time; a jump much larger than that was a teleport
	const float PlausibleDistance = PreviousVelocity.Size() * DeltaTime * 2.0f + 100.0f;
	const bool bTeleported = DistanceMoved > PlausibleDistance;

	PreviousVelocity = Velocity;
	PreviousLocation = Location;

	if (bTeleported)
	{
		GatheredSpeedLost = 0.0f;
		GatherElapsed = 0.0f;
		bGathering = false;
		return;
	}

	const FImpactSoundConfig& Config = SoundData->ImpactConfig;

	// A collision against something that does not move arrives as one big frame. A car struck from
	// behind is pushed along, so the same crash arrives as a run of middling frames instead, and
	// judging any one of them reports a fraction of what happened. Gather every frame pulling
	// harder than driving can and judge the collision on their total.
	const float GatherFloor = Config.MinGatherAcceleration * DeltaTime;

	if (SpeedLost >= GatherFloor)
	{
		if (!bGathering)
		{
			bGathering = true;
			GatheredLocation = Location;
		}

		GatheredSpeedLost += SpeedLost;
		GatherElapsed += DeltaTime;

		// still in contact and there is room left in the window, so wait and see how big this gets
		if (GatherElapsed < Config.ImpactGatherWindow)
		{
			return;
		}
	}
	else if (!bGathering)
	{
		return;
	}

	// contact ended, or the window is full: this is the whole event. Whether it is actually heard
	// is ReportImpact's call, so that this route and the hit-event route follow the same rule.
	ReportImpact(GatheredLocation, GatheredSpeedLost);

	GatheredSpeedLost = 0.0f;
	GatherElapsed = 0.0f;
	bGathering = false;
}
