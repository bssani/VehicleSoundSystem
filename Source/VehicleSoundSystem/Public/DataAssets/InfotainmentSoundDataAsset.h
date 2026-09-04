#pragma once

#include "CoreMinimal.h"
#include "DataAssets/VehicleSoundDataAsset.h"
#include "InfotainmentSoundDataAsset.generated.h"

/** Single track entry for the music playlist */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FMusicTrackEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
	FText TrackName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
	FText ArtistName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
	TObjectPtr<USoundBase> TrackSound;
};

/**
 * Data asset for infotainment sounds.
 * Includes widget UI feedback sounds and a music playlist.
 */
UCLASS(BlueprintType)
class VEHICLESOUNDSYSTEM_API UInfotainmentSoundDataAsset : public UVehicleSoundDataAsset
{
	GENERATED_BODY()

public:
	UInfotainmentSoundDataAsset()
	{
		Category = EVehicleSoundCategory::Infotainment;
	}

	/** Sound played on widget touch/press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Sounds")
	TObjectPtr<USoundBase> TouchFeedbackSound;

	/** Sound played on widget navigation/focus change */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Sounds")
	TObjectPtr<USoundBase> NavigationSound;

	/** Sound played on notification popup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Sounds")
	TObjectPtr<USoundBase> NotificationSound;

	/** Sound played on error/invalid action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Sounds")
	TObjectPtr<USoundBase> ErrorSound;

	/** Music playlist for the in-vehicle music player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music Player")
	TArray<FMusicTrackEntry> MusicPlaylist;
};
