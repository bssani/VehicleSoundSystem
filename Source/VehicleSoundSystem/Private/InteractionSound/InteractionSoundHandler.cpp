#include "InteractionSound/InteractionSoundHandler.h"
#include "Kismet/GameplayStatics.h"
#include "VehicleSoundSystemModule.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UInteractionSoundHandler::Initialize(AActor* InOwner, UInteractionSoundDataAsset* InDataAsset)
{
	OwningActor = InOwner;
	SoundData = InDataAsset;
}

void UInteractionSoundHandler::PlaySound(EInteractionSoundType Type)
{
	if (!SoundData || !OwningActor)
	{
		return;
	}

	TObjectPtr<USoundBase>* FoundSound = SoundData->InteractionSounds.Find(Type);
	if (!FoundSound || !(*FoundSound))
	{
		UE_LOG(LogVehicleSoundSystem, Verbose, TEXT("InteractionSoundHandler: No sound assigned for interaction type %d"), static_cast<int32>(Type));
		return;
	}

	if (SoundData->Attenuation)
	{
		UGameplayStatics::PlaySoundAtLocation(
			OwningActor,
			*FoundSound,
			OwningActor->GetActorLocation(),
			FRotator::ZeroRotator,
			VolumeMultiplier,
			1.0f,
			0.0f,
			SoundData->Attenuation,
			SoundData->Concurrency
		);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(
			OwningActor,
			*FoundSound,
			OwningActor->GetActorLocation(),
			VolumeMultiplier
		);
	}
}

void UInteractionSoundHandler::StartRepeatingSound(EInteractionSoundType Type, float IntervalSeconds)
{
	if (!OwningActor)
	{
		return;
	}

	StopRepeatingSound(Type);

	UWorld* World = OwningActor->GetWorld();
	if (!World)
	{
		return;
	}

	FTimerHandle& Handle = RepeatingTimerHandles.FindOrAdd(Type);
	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &UInteractionSoundHandler::PlaySound, Type);
	World->GetTimerManager().SetTimer(Handle, Delegate, IntervalSeconds, true, 0.0f);
}

void UInteractionSoundHandler::StopRepeatingSound(EInteractionSoundType Type)
{
	if (!OwningActor)
	{
		return;
	}

	FTimerHandle* Handle = RepeatingTimerHandles.Find(Type);
	if (Handle && Handle->IsValid())
	{
		UWorld* World = OwningActor->GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(*Handle);
		}
	}
	RepeatingTimerHandles.Remove(Type);
}

void UInteractionSoundHandler::StopAllRepeatingSounds()
{
	if (!OwningActor)
	{
		return;
	}

	UWorld* World = OwningActor->GetWorld();
	if (World)
	{
		for (auto& Pair : RepeatingTimerHandles)
		{
			if (Pair.Value.IsValid())
			{
				World->GetTimerManager().ClearTimer(Pair.Value);
			}
		}
	}
	RepeatingTimerHandles.Empty();
}
