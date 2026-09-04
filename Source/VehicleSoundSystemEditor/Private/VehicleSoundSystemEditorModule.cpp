#include "VehicleSoundSystemEditorModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FVehicleSoundSystemEditorModule"

void FVehicleSoundSystemEditorModule::StartupModule()
{
}

void FVehicleSoundSystemEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVehicleSoundSystemEditorModule, VehicleSoundSystemEditor)
