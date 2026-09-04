#include "ImpactSound/ImpactSoundHandler.h"
#include "VehicleSoundSystemModule.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

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

	OwningActor = nullptr;
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

	ReportImpact(Hit.ImpactPoint, ClosingSpeed);
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
		SoundData->Attenuation,
		SoundData->Concurrency);

	UE_LOG(LogVehicleSoundSystem, Verbose, TEXT("Impact at %.0f cm/s (severity %.2f) on %s"),
		ImpactSpeed, Severity, *OwningActor->GetName());
}
