#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "DataAssets/InfotainmentSoundDataAsset.h"
#include "MusicPlayerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrackChanged, int32, TrackIndex, const FMusicTrackEntry&, TrackEntry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackStateChanged, bool, bIsPlaying);

/**
 * In-vehicle music player component.
 * Manages a playlist from InfotainmentSoundDataAsset with play/pause/next/previous/shuffle.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class VEHICLESOUNDSYSTEM_API UMusicPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMusicPlayerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Initialize the music player with a data asset */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Music")
	void InitializePlaylist(UInfotainmentSoundDataAsset* InDataAsset);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Music")
	void Play();

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Music")
	void Pause();

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Music")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Music")
	void NextTrack();

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Music")
	void PreviousTrack();

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Music")
	void SetShuffle(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Music")
	void SetVolume(float InVolume);

	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|Music")
	bool IsPlaying() const { return bIsPlaying; }

	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|Music")
	int32 GetCurrentTrackIndex() const { return CurrentTrackIndex; }

	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|Music")
	int32 GetTrackCount() const;

	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|Music")
	FMusicTrackEntry GetCurrentTrackInfo() const;

	UPROPERTY(BlueprintAssignable, Category = "Vehicle Sound|Music")
	FOnTrackChanged OnTrackChanged;

	UPROPERTY(BlueprintAssignable, Category = "Vehicle Sound|Music")
	FOnPlaybackStateChanged OnPlaybackStateChanged;

private:
	void PlayTrackAtIndex(int32 Index);

	UFUNCTION()
	void OnAudioFinished();

	UPROPERTY()
	TObjectPtr<UInfotainmentSoundDataAsset> PlaylistData;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComp;

	int32 CurrentTrackIndex = 0;
	bool bIsPlaying = false;
	bool bShuffle = false;
	float MusicVolume = 1.0f;
};
