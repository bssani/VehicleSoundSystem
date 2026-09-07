#include "DynamicSound/DynamicSoundLayer.h"
#include "Components/VehicleSoundComponent.h"
#include "VehicleSoundSystemModule.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "MetasoundSource.h"

void UDynamicSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	OwningComponent = InOwner;
	DataAsset = InDataAsset;
}

void UDynamicSoundLayer::Activate()
{
	if (AudioComponent && !bIsActive)
	{
		AudioComponent->Play();
		bIsActive = true;
	}
}

void UDynamicSoundLayer::Deactivate()
{
	if (AudioComponent && bIsActive)
	{
		AudioComponent->Stop();
		bIsActive = false;
	}
}

void UDynamicSoundLayer::RestartIfStopped()
{
	if (!bIsActive || !AudioComponent || AudioComponent->IsPlaying())
	{
		return;
	}

	if (!bReportedSourceEnded)
	{
		bReportedSourceEnded = true;

		UE_LOG(LogVehicleSoundSystem, Warning,
			TEXT("DynamicSoundLayer: '%s' ended on its own, leaving layer type %d silent. Restarting it. ")
			TEXT("A continuous layer wants a source that runs until stopped; a MetaSound carrying the ")
			TEXT("UE.Source.OneShot interface ends when its graph triggers On Finished."),
			AudioComponent->Sound ? *AudioComponent->Sound->GetName() : TEXT("(none)"),
			static_cast<int32>(GetLayerType()));
	}

	AudioComponent->Play();
}

void UDynamicSoundLayer::SetVolume(float InVolume)
{
	Volume = FMath::Clamp(InVolume, 0.0f, 1.0f);
	if (AudioComponent)
	{
		AudioComponent->SetVolumeMultiplier(Volume);
	}
}

void UDynamicSoundLayer::BeginDestroy()
{
	Deactivate();
	if (AudioComponent)
	{
		AudioComponent->DestroyComponent();
		AudioComponent = nullptr;
	}
	Super::BeginDestroy();
}

UAudioComponent* UDynamicSoundLayer::CreateAudioComponent(USoundBase* Sound, USoundAttenuation* AttenuationOverride, USoundConcurrency* ConcurrencyOverride)
{
	if (!Sound)
	{
		UE_LOG(LogVehicleSoundSystem, Warning, TEXT("DynamicSoundLayer: No MetaSound source assigned for layer type %d. Sound will be silent."), static_cast<int32>(GetLayerType()));
		return nullptr;
	}

	if (!OwningComponent || !OwningComponent->GetOwner())
	{
		UE_LOG(LogVehicleSoundSystem, Error, TEXT("DynamicSoundLayer: No valid owning component/actor."));
		return nullptr;
	}

	AActor* Owner = OwningComponent->GetOwner();
	UAudioComponent* NewAudioComp = NewObject<UAudioComponent>(Owner);
	if (!NewAudioComp)
	{
		return nullptr;
	}

	bSourceIsMetaSound = Sound->IsA<UMetaSoundSource>();

	// These layers run for as long as the vehicle exists. Handed a one-shot they play it once and
	// fall silent for the rest of the session, which looks like the layer never worked at all.
	// Environment cues are the usual trap: they sound like loops but are a fixed length.
	// MetaSound graphs run until stopped, and report a placeholder duration rather than looping,
	// so they are exempt from the check.
	if (!bSourceIsMetaSound && !Sound->IsLooping())
	{
		UE_LOG(LogVehicleSoundSystem, Warning,
			TEXT("DynamicSoundLayer: '%s' is %.1fs long and does not loop, so layer type %d will go quiet once it ends. Continuous layers need a looping sound."),
			*Sound->GetName(), Sound->GetDuration(), static_cast<int32>(GetLayerType()));
	}

	NewAudioComp->SetSound(Sound);
	NewAudioComp->bAutoActivate = false;
	NewAudioComp->bAutoDestroy = false;
	NewAudioComp->SetVolumeMultiplier(Volume);

	// Apply audio settings from data asset
	if (DataAsset)
	{
		if (DataAsset->SoundClass)
		{
			NewAudioComp->SoundClassOverride = DataAsset->SoundClass;
		}
		USoundAttenuation* Attenuation = AttenuationOverride ? AttenuationOverride : DataAsset->Attenuation.Get();

		if (Attenuation)
		{
			NewAudioComp->AttenuationSettings = Attenuation;
		}
		USoundConcurrency* Concurrency = ConcurrencyOverride ? ConcurrencyOverride : DataAsset->Concurrency.Get();

		if (Concurrency)
		{
			NewAudioComp->ConcurrencySet.Add(Concurrency);
		}
	}

	NewAudioComp->RegisterComponent();
	NewAudioComp->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	return NewAudioComp;
}

void UDynamicSoundLayer::SetPlaybackPitch(float Pitch)
{
	if (AudioComponent)
	{
		AudioComponent->SetPitchMultiplier(FMath::Max(Pitch, KINDA_SMALL_NUMBER));
	}
}

void UDynamicSoundLayer::SwapSound(USoundBase* NewSound)
{
	if (!AudioComponent || !NewSound || AudioComponent->Sound == NewSound)
	{
		return;
	}

	const bool bWasPlaying = AudioComponent->IsPlaying();

	AudioComponent->SetSound(NewSound);

	if (bWasPlaying)
	{
		AudioComponent->Play();
	}
}

FName UDynamicSoundLayer::ResolveParameterName(FName ParameterName) const
{
	if (DataAsset)
	{
		if (const FName* Override = DataAsset->ParameterNameOverrides.Find(ParameterName))
		{
			return *Override;
		}
	}

	return ParameterName;
}

void UDynamicSoundLayer::SetMetaSoundParameter(FName ParameterName, float Value)
{
	if (AudioComponent)
	{
		AudioComponent->SetFloatParameter(ResolveParameterName(ParameterName), Value);
	}
}

void UDynamicSoundLayer::SetMetaSoundIntParameter(FName ParameterName, int32 Value)
{
	if (AudioComponent)
	{
		AudioComponent->SetIntParameter(ResolveParameterName(ParameterName), Value);
	}
}
