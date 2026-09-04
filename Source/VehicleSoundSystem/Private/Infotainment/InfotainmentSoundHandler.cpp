#include "Infotainment/InfotainmentSoundHandler.h"
#include "Kismet/GameplayStatics.h"
#include "VehicleSoundSystemModule.h"

void UInfotainmentSoundHandler::Initialize(UInfotainmentSoundDataAsset* InDataAsset)
{
	SoundData = InDataAsset;
}

void UInfotainmentSoundHandler::PlayTouchFeedback()
{
	if (SoundData)
	{
		PlaySound2D(SoundData->TouchFeedbackSound);
	}
}

void UInfotainmentSoundHandler::PlayNavigationSound()
{
	if (SoundData)
	{
		PlaySound2D(SoundData->NavigationSound);
	}
}

void UInfotainmentSoundHandler::PlayNotification()
{
	if (SoundData)
	{
		PlaySound2D(SoundData->NotificationSound);
	}
}

void UInfotainmentSoundHandler::PlayErrorSound()
{
	if (SoundData)
	{
		PlaySound2D(SoundData->ErrorSound);
	}
}

void UInfotainmentSoundHandler::PlaySound2D(USoundBase* Sound)
{
	if (!Sound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(this, Sound, VolumeMultiplier);
}
