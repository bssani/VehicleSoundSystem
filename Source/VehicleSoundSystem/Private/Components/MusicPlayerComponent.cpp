#include "Components/MusicPlayerComponent.h"
#include "VehicleSoundSystemModule.h"

UMusicPlayerComponent::UMusicPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMusicPlayerComponent::BeginPlay()
{
	Super::BeginPlay();

	AudioComp = NewObject<UAudioComponent>(GetOwner());
	if (AudioComp)
	{
		AudioComp->bAutoActivate = false;
		AudioComp->bAutoDestroy = false;
		AudioComp->bIsUISound = true;
		AudioComp->RegisterComponent();
		AudioComp->OnAudioFinished.AddDynamic(this, &UMusicPlayerComponent::OnAudioFinished);
	}
}

void UMusicPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Stop();
	if (AudioComp)
	{
		AudioComp->DestroyComponent();
		AudioComp = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void UMusicPlayerComponent::InitializePlaylist(UInfotainmentSoundDataAsset* InDataAsset)
{
	PlaylistData = InDataAsset;
	CurrentTrackIndex = 0;
}

void UMusicPlayerComponent::Play()
{
	if (!PlaylistData || PlaylistData->MusicPlaylist.Num() == 0)
	{
		UE_LOG(LogVehicleSoundSystem, Warning, TEXT("MusicPlayer: No playlist data or empty playlist."));
		return;
	}

	if (AudioComp && AudioComp->GetSound() && !bIsPlaying)
	{
		// Resume paused playback
		AudioComp->SetPaused(false);
		bIsPlaying = true;
		OnPlaybackStateChanged.Broadcast(true);
		return;
	}

	PlayTrackAtIndex(CurrentTrackIndex);
}

void UMusicPlayerComponent::Pause()
{
	if (AudioComp && bIsPlaying)
	{
		AudioComp->SetPaused(true);
		bIsPlaying = false;
		OnPlaybackStateChanged.Broadcast(false);
	}
}

void UMusicPlayerComponent::Stop()
{
	if (AudioComp)
	{
		AudioComp->Stop();
	}
	bIsPlaying = false;
	OnPlaybackStateChanged.Broadcast(false);
}

void UMusicPlayerComponent::NextTrack()
{
	if (!PlaylistData || PlaylistData->MusicPlaylist.Num() == 0)
	{
		return;
	}

	if (bShuffle)
	{
		CurrentTrackIndex = FMath::RandRange(0, PlaylistData->MusicPlaylist.Num() - 1);
	}
	else
	{
		CurrentTrackIndex = (CurrentTrackIndex + 1) % PlaylistData->MusicPlaylist.Num();
	}

	if (bIsPlaying)
	{
		PlayTrackAtIndex(CurrentTrackIndex);
	}
}

void UMusicPlayerComponent::PreviousTrack()
{
	if (!PlaylistData || PlaylistData->MusicPlaylist.Num() == 0)
	{
		return;
	}

	CurrentTrackIndex = (CurrentTrackIndex - 1 + PlaylistData->MusicPlaylist.Num()) % PlaylistData->MusicPlaylist.Num();

	if (bIsPlaying)
	{
		PlayTrackAtIndex(CurrentTrackIndex);
	}
}

void UMusicPlayerComponent::SetShuffle(bool bEnable)
{
	bShuffle = bEnable;
}

void UMusicPlayerComponent::SetVolume(float InVolume)
{
	MusicVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
	if (AudioComp)
	{
		AudioComp->SetVolumeMultiplier(MusicVolume);
	}
}

int32 UMusicPlayerComponent::GetTrackCount() const
{
	return PlaylistData ? PlaylistData->MusicPlaylist.Num() : 0;
}

FMusicTrackEntry UMusicPlayerComponent::GetCurrentTrackInfo() const
{
	if (PlaylistData && PlaylistData->MusicPlaylist.IsValidIndex(CurrentTrackIndex))
	{
		return PlaylistData->MusicPlaylist[CurrentTrackIndex];
	}
	return FMusicTrackEntry();
}

void UMusicPlayerComponent::PlayTrackAtIndex(int32 Index)
{
	if (!PlaylistData || !AudioComp)
	{
		return;
	}

	if (!PlaylistData->MusicPlaylist.IsValidIndex(Index))
	{
		UE_LOG(LogVehicleSoundSystem, Warning, TEXT("MusicPlayer: Track index %d out of range."), Index);
		return;
	}

	const FMusicTrackEntry& Track = PlaylistData->MusicPlaylist[Index];
	if (!Track.TrackSound)
	{
		UE_LOG(LogVehicleSoundSystem, Warning, TEXT("MusicPlayer: Track at index %d has no sound."), Index);
		NextTrack();
		return;
	}

	AudioComp->Stop();
	AudioComp->SetSound(Track.TrackSound);
	AudioComp->SetVolumeMultiplier(MusicVolume);
	AudioComp->Play();

	CurrentTrackIndex = Index;
	bIsPlaying = true;
	OnTrackChanged.Broadcast(Index, Track);
	OnPlaybackStateChanged.Broadcast(true);
}

void UMusicPlayerComponent::OnAudioFinished()
{
	if (bIsPlaying)
	{
		NextTrack();
		if (PlaylistData && PlaylistData->MusicPlaylist.Num() > 0)
		{
			PlayTrackAtIndex(CurrentTrackIndex);
		}
	}
}
