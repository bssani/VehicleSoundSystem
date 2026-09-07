#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enums/VehicleSoundEnums.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Subsystem/VehicleSoundSubsystem.h"
#include "DataAssets/VehicleSoundPreset.h"
#include "DynamicSound/DynamicSoundLayer.h"
#include "InteractionSound/InteractionSoundHandler.h"
#include "Infotainment/InfotainmentSoundHandler.h"
#include "ImpactSound/ImpactSoundHandler.h"
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

	/** Maps the physical material under the wheels onto a tyre surface.
	 *
	 *  Surface types are defined per project, so there is no sensible default beyond asphalt.
	 *  Until a project fills this in every surface sounds like tarmac, which is wrong the moment
	 *  the car leaves the road. Take the surface from the worst-gripping wheel in contact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|Config")
	TMap<TEnumAsByte<EPhysicalSurface>, ETireSurfaceType> SurfaceTypeMapping;

	/** How close the listener has to be to count as sitting in this car. Measured to the camera,
	 *  so a cockpit view is inside and a chase camera is not, which is what a listener actually
	 *  hears. Wide enough to cover a head leaning around a cabin, short enough that the car in
	 *  front is never mistaken for your own */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|Config", meta = (Units = "cm", ClampMin = "0.0"))
	float InteriorListenerRadius = 250.0f;

	/** Slip a tyre carries before it is heard at all. Tyres slip a little whenever a car turns or
	 *  accelerates - that is how they make grip - and without a floor here every steering input
	 *  squeals. Only what exceeds this is treated as sliding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|Config", meta = (ClampMin = "0.0"))
	float TireSlipThreshold = 60.0f;

	/** Slip treated as a full slide, where tyre noise is at its loudest. Together with the
	 *  threshold this maps Chaos slip onto 0-1: raise it if tyres squeal too readily, lower it if
	 *  they stay quiet through a slide */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound|Config", meta = (ClampMin = "1.0"))
	float TireSlipReference = 300.0f;

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

	/** Reports a collision so it can be heard. Called automatically from the owner's hit events;
	 *  call it directly if the vehicle resolves its own collisions */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Impact")
	void ReportImpact(const FVector& Location, float ImpactSpeed);

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

	/** Sets the volume of one layer type. Silencing a single layer is how you find out which one
	 *  is making a noise; the console commands below drive this across every vehicle at once */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|Global")
	void SetLayerVolume(EDynamicSoundLayerType LayerType, float InVolume);

	/** Writes what each layer is playing and how loudly to the log */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Sound|State")
	void LogSoundState() const;

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

	/** Turns layers on and off by how far this vehicle is from the listener */
	void UpdateLOD();

	/** LOD applied last frame, so layers are only toggled when the tier actually changes */
	EVehicleSoundLOD CurrentLOD = EVehicleSoundLOD::Full;

	/** What the layers are being driven with this frame. Read-only and visible while playing, so a
	 *  sound that is behaving oddly can be checked against the numbers feeding it */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle Sound|State", meta = (AllowPrivateAccess = "true"))
	FVehicleSoundState CurrentState;

	UPROPERTY()
	TArray<TObjectPtr<UDynamicSoundLayer>> DynamicLayers;

	UPROPERTY()
	TObjectPtr<UInteractionSoundHandler> InteractionHandler;

	UPROPERTY()
	TObjectPtr<UImpactSoundHandler> ImpactHandler;

	UPROPERTY()
	TObjectPtr<UInfotainmentSoundHandler> InfotainmentHandler;

	UPROPERTY()
	TObjectPtr<UMusicPlayerComponent> MusicPlayer;

	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> CachedChaosVehicle;

	bool bInitialized = false;
};
