#include "VehicleSoundSystemModule.h"

DEFINE_LOG_CATEGORY(LogVehicleSoundSystem);

#define LOCTEXT_NAMESPACE "FVehicleSoundSystemModule"

void FVehicleSoundSystemModule::StartupModule()
{
	UE_LOG(LogVehicleSoundSystem, Log, TEXT("VehicleSoundSystem module started."));
}

void FVehicleSoundSystemModule::ShutdownModule()
{
	UE_LOG(LogVehicleSoundSystem, Log, TEXT("VehicleSoundSystem module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVehicleSoundSystemModule, VehicleSoundSystem)
