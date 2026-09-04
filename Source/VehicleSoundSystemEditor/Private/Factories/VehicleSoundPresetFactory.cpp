#include "Factories/VehicleSoundPresetFactory.h"
#include "DataAssets/VehicleSoundPreset.h"
#include "DataAssets/DynamicSoundDataAsset.h"
#include "DataAssets/InteractionSoundDataAsset.h"
#include "DataAssets/InfotainmentSoundDataAsset.h"
#include "AssetToolsModule.h"

// --- VehicleSoundPreset Factory ---

UVehicleSoundPresetFactory::UVehicleSoundPresetFactory()
{
	SupportedClass = UVehicleSoundPreset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UVehicleSoundPresetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UVehicleSoundPreset>(InParent, Class, Name, Flags);
}

FText UVehicleSoundPresetFactory::GetDisplayName() const
{
	return FText::FromString(TEXT("Vehicle Sound Preset"));
}

uint32 UVehicleSoundPresetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Sounds;
}

// --- DynamicSoundDataAsset Factory ---

UDynamicSoundDataAssetFactory::UDynamicSoundDataAssetFactory()
{
	SupportedClass = UDynamicSoundDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDynamicSoundDataAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UDynamicSoundDataAsset>(InParent, Class, Name, Flags);
}

FText UDynamicSoundDataAssetFactory::GetDisplayName() const
{
	return FText::FromString(TEXT("Dynamic Sound Data Asset"));
}

uint32 UDynamicSoundDataAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Sounds;
}

// --- InteractionSoundDataAsset Factory ---

UInteractionSoundDataAssetFactory::UInteractionSoundDataAssetFactory()
{
	SupportedClass = UInteractionSoundDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UInteractionSoundDataAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UInteractionSoundDataAsset>(InParent, Class, Name, Flags);
}

FText UInteractionSoundDataAssetFactory::GetDisplayName() const
{
	return FText::FromString(TEXT("Interaction Sound Data Asset"));
}

uint32 UInteractionSoundDataAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Sounds;
}

// --- InfotainmentSoundDataAsset Factory ---

UInfotainmentSoundDataAssetFactory::UInfotainmentSoundDataAssetFactory()
{
	SupportedClass = UInfotainmentSoundDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UInfotainmentSoundDataAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UInfotainmentSoundDataAsset>(InParent, Class, Name, Flags);
}

FText UInfotainmentSoundDataAssetFactory::GetDisplayName() const
{
	return FText::FromString(TEXT("Infotainment Sound Data Asset"));
}

uint32 UInfotainmentSoundDataAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Sounds;
}
