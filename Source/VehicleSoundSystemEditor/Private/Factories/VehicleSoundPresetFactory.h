#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "VehicleSoundPresetFactory.generated.h"

/**
 * Factory for creating VehicleSoundPreset assets from the editor content browser.
 * Right-click > Miscellaneous > Vehicle Sound Preset
 */
UCLASS()
class UVehicleSoundPresetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVehicleSoundPresetFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;
};

/** Factory for DynamicSoundDataAsset */
UCLASS()
class UDynamicSoundDataAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDynamicSoundDataAssetFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;
};

/** Factory for InteractionSoundDataAsset */
UCLASS()
class UInteractionSoundDataAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UInteractionSoundDataAssetFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;
};

/** Factory for InfotainmentSoundDataAsset */
UCLASS()
class UInfotainmentSoundDataAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UInfotainmentSoundDataAssetFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;
};
