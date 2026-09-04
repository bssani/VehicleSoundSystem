#include "Subsystem/VehicleSoundSubsystem.h"
#include "Components/VehicleSoundComponent.h"
#include "VehicleSoundSystemModule.h"

void UVehicleSoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MasterVolume = 1.0f;
	CategoryVolumes.Add(EVehicleSoundCategory::Dynamic, 1.0f);
	CategoryVolumes.Add(EVehicleSoundCategory::Interaction, 1.0f);
	CategoryVolumes.Add(EVehicleSoundCategory::Infotainment, 1.0f);

	UE_LOG(LogVehicleSoundSystem, Log, TEXT("VehicleSoundSubsystem initialized."));
}

void UVehicleSoundSubsystem::Deinitialize()
{
	ActiveVehicles.Empty();
	Super::Deinitialize();
}

EVehicleSoundLOD UVehicleSoundSubsystem::GetLODForDistance(float Distance) const
{
	if (Distance >= CullDistance)
	{
		return EVehicleSoundLOD::Culled;
	}

	if (Distance >= EngineOnlyDistance)
	{
		return EVehicleSoundLOD::EngineOnly;
	}

	if (Distance >= ReducedDistance)
	{
		return EVehicleSoundLOD::Reduced;
	}

	return EVehicleSoundLOD::Full;
}

void UVehicleSoundSubsystem::RegisterVehicle(UVehicleSoundComponent* Component)
{
	if (Component)
	{
		ActiveVehicles.AddUnique(Component);
		UE_LOG(LogVehicleSoundSystem, Verbose, TEXT("Registered vehicle sound component. Active count: %d"), ActiveVehicles.Num());
	}
}

void UVehicleSoundSubsystem::UnregisterVehicle(UVehicleSoundComponent* Component)
{
	ActiveVehicles.RemoveAll([Component](const TWeakObjectPtr<UVehicleSoundComponent>& Weak)
	{
		return !Weak.IsValid() || Weak.Get() == Component;
	});
	UE_LOG(LogVehicleSoundSystem, Verbose, TEXT("Unregistered vehicle sound component. Active count: %d"), ActiveVehicles.Num());
}

void UVehicleSoundSubsystem::SetMasterVolume(float InVolume)
{
	MasterVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
}

void UVehicleSoundSubsystem::SetCategoryVolume(EVehicleSoundCategory Category, float InVolume)
{
	CategoryVolumes.FindOrAdd(Category) = FMath::Clamp(InVolume, 0.0f, 1.0f);
}

float UVehicleSoundSubsystem::GetCategoryVolume(EVehicleSoundCategory Category) const
{
	const float* Found = CategoryVolumes.Find(Category);
	return Found ? *Found : 1.0f;
}
