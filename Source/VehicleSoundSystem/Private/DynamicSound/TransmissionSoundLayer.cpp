#include "DynamicSound/TransmissionSoundLayer.h"
#include "Components/VehicleSoundComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VehicleSoundSystemModule.h"

void UTransmissionSoundLayer::Initialize(UVehicleSoundComponent* InOwner, UDynamicSoundDataAsset* InDataAsset)
{
	Super::Initialize(InOwner, InDataAsset);

	if (DataAsset)
	{
		AudioComponent = CreateAudioComponent(DataAsset->TransmissionConfig.MetaSoundSource);
	}
}

void UTransmissionSoundLayer::Update(float DeltaTime, const FVehicleSoundState& State)
{
	if (!AudioComponent || !bIsActive)
	{
		return;
	}

	SetMetaSoundParameter(FName("RPM"), State.RPM);
	SetMetaSoundIntParameter(FName("Gear"), State.CurrentGear);
	SetMetaSoundParameter(FName("EngineLoad"), State.EngineLoad);

	// Detect gear change and play one-shot shift sound
	if (State.CurrentGear != LastGear && LastGear != 0)
	{
		PlayGearShiftSound();
	}
	LastGear = State.CurrentGear;

	if (DataAsset && DataAsset->TransmissionConfig.RPMToWhineVolume)
	{
		float CurveVolume = DataAsset->TransmissionConfig.RPMToWhineVolume->GetFloatValue(State.RPM);
		AudioComponent->SetVolumeMultiplier(Volume * CurveVolume);
	}
}

void UTransmissionSoundLayer::PlayGearShiftSound()
{
	if (!DataAsset || !OwningComponent || !OwningComponent->GetOwner())
	{
		return;
	}

	const TArray<TObjectPtr<USoundWave>>& ShiftSounds = DataAsset->TransmissionConfig.GearShiftSounds;
	if (ShiftSounds.Num() == 0)
	{
		return;
	}

	int32 Index = FMath::RandRange(0, ShiftSounds.Num() - 1);
	if (ShiftSounds[Index])
	{
		UGameplayStatics::PlaySoundAtLocation(
			OwningComponent->GetOwner(),
			ShiftSounds[Index],
			OwningComponent->GetOwner()->GetActorLocation(),
			Volume
		);
	}
}
