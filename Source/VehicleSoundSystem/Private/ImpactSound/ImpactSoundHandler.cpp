#include "ImpactSound/ImpactSoundHandler.h"
#include "VehicleSoundSystemModule.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"

void UImpactSoundHandler::Initialize(AActor* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	OwningActor = InOwner;
	SoundData = InDataAsset;

	if (OwningActor)
	{
		OwningActor->OnActorHit.AddDynamic(this, &UImpactSoundHandler::HandleActorHit);
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

	if (Config.ImpactSounds.Num() == 0 || ImpactSpeed < Config.MinImpactSpeed)
	{
		return;
	}

	const UWorld* World = OwningActor->GetWorld();

	if (!World)
	{
		return;
	}

	// Chaos reports contact every frame while two bodies stay touching, so without this a scrape
	// along a wall fires a one-shot per frame
	const double Now = World->GetTimeSeconds();

	if (Now - LastImpactTime < Config.MinTimeBetweenImpacts)
	{
		return;
	}

	LastImpactTime = Now;

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
