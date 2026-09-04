#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enums/VehicleSoundEnums.h"
#include "DataAssets/VehicleSoundPreset.h"
#include "DynamicSound/DynamicSoundLayer.h"
#include "InteractionSound/InteractionSoundHandler.h"
#include "Infotainment/InfotainmentSoundHandler.h"
#include "VehicleSoundComponent.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UMusicPlayerComponent;

/**
 * Main component attached to vehicle actors.
 * Orchestrates all three sound categories: Dynamic, Interaction, Infotainment.
 * Supports auto-detection from Chaos Vehicle or manual parameter input.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class VEHICLESOUNDSYSTEM_API UVehicleSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVehicleSoundComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Configuration ---

	/** Sound preset containing all three data assets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|Config")
	TObjectPtr<UVehicleSoundPreset> SoundPreset;

	/** Vehicle powertrain type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|Config")
	EVehiclePowertrainType PowertrainType = EVehiclePowertrainType::ICE;

	/** If true, auto-detects ChaosVehicleMovementComponent and reads RPM/Speed/Gear from it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|Config")
	bool bAutoDetectFromChaosVehicle = true;

	// --- Manual State Input (use when bAutoDetectFromChaosVehicle is false) ---

	/** Manually update the full vehicle sound state */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetVehicleSoundState(const FVehicleSoundState& InState);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetEngineRunning(bool bRunning);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetCurrentRPM(float InRPM);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetCurrentSpeed(float InSpeedKmh);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetThrottleInput(float InThrottle);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetBrakeInput(float InBrake);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetSteeringInput(float InSteering);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetCurrentGear(int32 InGear);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetTireSurface(ETireSurfaceType InSurface);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void SetWindowOpenAmount(float InAmount);

	// --- Interaction ---

	/** Play a one-shot interaction sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Interaction")
	void PlayInteractionSound(EInteractionSoundType Type);

	/** Start turn signal (repeating tick sound) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Interaction")
	void StartTurnSignal(float TickInterval = 0.5f);

	/** Stop turn signal */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Interaction")
	void StopTurnSignal();

	// --- Infotainment ---

	/** Play UI touch feedback sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Infotainment")
	void PlayTouchFeedback();

	/** Play notification sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Infotainment")
	void PlayNotification();

	/** Get the music player component (created on demand) */
	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|Infotainment")
	UMusicPlayerComponent* GetMusicPlayer() const { return MusicPlayer; }

	// --- Volume ---

	/** Set volume for a specific sound category */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Volume")
	void SetCategoryVolume(EVehicleSoundCategory Category, float InVolume);

	// --- State Access ---

	UFUNCTION(BlueprintPure, Category = "Vehicle Sound|State")
	const FVehicleSoundState& GetCurrentSoundState() const { return CurrentState; }

private:
	/** Initialize all sound systems from the preset */
	void InitializeFromPreset();

	/** Create dynamic sound layers based on powertrain type */
	void CreateDynamicLayers();

	/** Gather vehicle state from ChaosVehicleMovementComponent */
	void GatherStateFromChaosVehicle();

	/** Update all dynamic layers with current state */
	void UpdateDynamicLayers(float DeltaTime);

	UPROPERTY()
	FVehicleSoundState CurrentState;

	UPROPERTY()
	TArray<TObjectPtr<UDynamicSoundLayer>> DynamicLayers;

	UPROPERTY()
	TObjectPtr<UInteractionSoundHandler> InteractionHandler;

	UPROPERTY()
	TObjectPtr<UInfotainmentSoundHandler> InfotainmentHandler;

	UPROPERTY()
	TObjectPtr<UMusicPlayerComponent> MusicPlayer;

	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> CachedChaosVehicle;

	bool bInitialized = false;
};
