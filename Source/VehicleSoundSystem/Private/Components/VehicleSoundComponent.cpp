#include "Components/VehicleSoundComponent.h"
#include "ImpactSound/ImpactSoundHandler.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/MusicPlayerComponent.h"
#include "Subsystem/VehicleSoundSubsystem.h"
#include "DynamicSound/EngineSoundLayer.h"
#include "DynamicSound/EVMotorSoundLayer.h"
#include "DynamicSound/ExhaustSoundLayer.h"
#include "DynamicSound/TireSoundLayer.h"
#include "DynamicSound/WindSoundLayer.h"
#include "DynamicSound/TransmissionSoundLayer.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "VehicleSoundSystemModule.h"
#include "Engine/GameInstance.h"

UVehicleSoundComponent::UVehicleSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UVehicleSoundComponent::BeginPlay()
{
	Super::BeginPlay();

	// Auto-detect Chaos Vehicle
	if (bAutoDetectFromChaosVehicle && GetOwner())
	{
		CachedChaosVehicle = GetOwner()->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
		if (CachedChaosVehicle)
		{
			UE_LOG(LogVehicleSoundSystem, Log, TEXT("Auto-detected ChaosVehicleMovementComponent on %s"), *GetOwner()->GetName());
		}
		else
		{
			UE_LOG(LogVehicleSoundSystem, Log, TEXT("No ChaosVehicleMovementComponent found on %s. Using manual input."), *GetOwner()->GetName());
		}
	}

	CurrentState.PowertrainType = PowertrainType;
	InitializeFromPreset();

	// Register with subsystem
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UVehicleSoundSubsystem* Subsystem = GI->GetSubsystem<UVehicleSoundSubsystem>())
		{
			Subsystem->RegisterVehicle(this);
		}
	}
}

void UVehicleSoundComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from subsystem
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		if (UVehicleSoundSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UVehicleSoundSubsystem>())
		{
			Subsystem->UnregisterVehicle(this);
		}
	}

	// Deactivate all layers
	for (UDynamicSoundLayer* Layer : DynamicLayers)
	{
		if (Layer)
		{
			Layer->Deactivate();
		}
	}
	DynamicLayers.Empty();

	if (InteractionHandler)
	{
		InteractionHandler->StopAllRepeatingSounds();
	}

	// unhooks the owner's hit delegate
	if (ImpactHandler)
	{
		ImpactHandler->Shutdown();
	}

	Super::EndPlay(EndPlayReason);
}

void UVehicleSoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitialized)
	{
		return;
	}

	// Gather state from Chaos Vehicle if available
	if (bAutoDetectFromChaosVehicle && CachedChaosVehicle)
	{
		GatherStateFromChaosVehicle();
	}

	UpdateLOD();
	UpdateDynamicLayers(DeltaTime);

	// the scrape has to be told when contact stopped; nothing reports the absence of a hit
	if (ImpactHandler)
	{
		ImpactHandler->Tick(DeltaTime);
	}
}

void UVehicleSoundComponent::UpdateLOD()
{
	const UWorld* World = GetWorld();
	AActor* Owner = GetOwner();

	if (!World || !Owner)
	{
		return;
	}

	UVehicleSoundSubsystem* Subsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UVehicleSoundSubsystem>()
		: nullptr;

	if (!Subsystem)
	{
		return;
	}

	// measured against the camera rather than the pawn, which is what the listener follows and
	// is the difference that matters in VR
	const APlayerController* PC = World->GetFirstPlayerController();

	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const float Distance = FVector::Dist(PC->PlayerCameraManager->GetCameraLocation(), Owner->GetActorLocation());

	// the same measurement answers which mix to play, so it is taken here rather than repeated
	CurrentState.bListenerInside = Distance <= InteriorListenerRadius;

	const EVehicleSoundLOD NewLOD = Subsystem->GetLODForDistance(Distance);

	if (NewLOD == CurrentLOD)
	{
		return;
	}

	CurrentLOD = NewLOD;

	for (UDynamicSoundLayer* Layer : DynamicLayers)
	{
		if (!Layer)
		{
			continue;
		}

		bool bShouldRun = false;

		switch (CurrentLOD)
		{
		case EVehicleSoundLOD::Full:
			bShouldRun = true;
			break;

		case EVehicleSoundLOD::Reduced:
			// wind and gearbox whine are cabin detail; they don't carry across a track
			bShouldRun = Layer->GetLayerType() == EDynamicSoundLayerType::Engine
				|| Layer->GetLayerType() == EDynamicSoundLayerType::EVMotor
				|| Layer->GetLayerType() == EDynamicSoundLayerType::Exhaust
				|| Layer->GetLayerType() == EDynamicSoundLayerType::Tire;
			break;

		case EVehicleSoundLOD::EngineOnly:
			bShouldRun = Layer->GetLayerType() == EDynamicSoundLayerType::Engine
				|| Layer->GetLayerType() == EDynamicSoundLayerType::EVMotor;
			break;

		case EVehicleSoundLOD::Culled:
			bShouldRun = false;
			break;
		}

		if (bShouldRun && !Layer->IsActive())
		{
			Layer->Activate();
		}
		else if (!bShouldRun && Layer->IsActive())
		{
			Layer->Deactivate();
		}
	}
}

void UVehicleSoundComponent::InitializeFromPreset()
{
	if (!SoundPreset)
	{
		UE_LOG(LogVehicleSoundSystem, Warning, TEXT("VehicleSoundComponent: No SoundPreset assigned on %s."), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
		return;
	}

	// Dynamic layers
	if (SoundPreset->DynamicSoundData)
	{
		CreateDynamicLayers();
	}

	// Impact handler. Lives on the dynamic asset because collisions are part of driving, and it
	// hooks the owner's hit events itself
	if (SoundPreset->DynamicSoundData)
	{
		ImpactHandler = NewObject<UImpactSoundHandler>(this);
		ImpactHandler->Initialize(GetOwner(), SoundPreset->DynamicSoundData);
	}

	// Interaction handler
	if (SoundPreset->InteractionSoundData)
	{
		InteractionHandler = NewObject<UInteractionSoundHandler>(this);
		InteractionHandler->Initialize(GetOwner(), SoundPreset->InteractionSoundData);
	}

	// Infotainment handler
	if (SoundPreset->InfotainmentSoundData)
	{
		InfotainmentHandler = NewObject<UInfotainmentSoundHandler>(this);
		InfotainmentHandler->Initialize(SoundPreset->InfotainmentSoundData);

		// Create music player component if playlist exists
		if (SoundPreset->InfotainmentSoundData->MusicPlaylist.Num() > 0)
		{
			MusicPlayer = NewObject<UMusicPlayerComponent>(GetOwner());
			if (MusicPlayer)
			{
				MusicPlayer->RegisterComponent();
				MusicPlayer->InitializePlaylist(SoundPreset->InfotainmentSoundData);
			}
		}
	}

	bInitialized = true;
	UE_LOG(LogVehicleSoundSystem, Log, TEXT("VehicleSoundComponent initialized with preset '%s' on %s."),
		*SoundPreset->PresetName.ToString(), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UVehicleSoundComponent::CreateDynamicLayers()
{
	UDynamicSoundDataAsset* DynData = SoundPreset->DynamicSoundData;

	// Engine/Motor layer
	if (PowertrainType == EVehiclePowertrainType::ICE)
	{
		UEngineSoundLayer* EngineLayer = NewObject<UEngineSoundLayer>(this);
		EngineLayer->Initialize(this, DynData);
		EngineLayer->Activate();
		DynamicLayers.Add(EngineLayer);

		// Exhaust layer (ICE only)
		UExhaustSoundLayer* ExhaustLayer = NewObject<UExhaustSoundLayer>(this);
		ExhaustLayer->Initialize(this, DynData);
		ExhaustLayer->Activate();
		DynamicLayers.Add(ExhaustLayer);
	}
	else if (PowertrainType == EVehiclePowertrainType::EV)
	{
		UEVMotorSoundLayer* EVLayer = NewObject<UEVMotorSoundLayer>(this);
		EVLayer->Initialize(this, DynData);
		EVLayer->Activate();
		DynamicLayers.Add(EVLayer);
	}

	// Transmission layer (both ICE and EV)
	UTransmissionSoundLayer* TransLayer = NewObject<UTransmissionSoundLayer>(this);
	TransLayer->Initialize(this, DynData);
	TransLayer->Activate();
	DynamicLayers.Add(TransLayer);

	// Tire layer
	UTireSoundLayer* TireLayer = NewObject<UTireSoundLayer>(this);
	TireLayer->Initialize(this, DynData);
	TireLayer->Activate();
	DynamicLayers.Add(TireLayer);

	// Wind layer
	UWindSoundLayer* WindLayer = NewObject<UWindSoundLayer>(this);
	WindLayer->Initialize(this, DynData);
	WindLayer->Activate();
	DynamicLayers.Add(WindLayer);

	UE_LOG(LogVehicleSoundSystem, Log, TEXT("Created %d dynamic sound layers for %s powertrain."),
		DynamicLayers.Num(), PowertrainType == EVehiclePowertrainType::ICE ? TEXT("ICE") : TEXT("EV"));
}

void UVehicleSoundComponent::GatherStateFromChaosVehicle()
{
	if (!CachedChaosVehicle)
	{
		return;
	}

	CurrentState.RPM = CachedChaosVehicle->GetEngineRotationSpeed();
	CurrentState.Speed = FMath::Abs(CachedChaosVehicle->GetForwardSpeed()) * 0.036f; // cm/s to km/h
	CurrentState.ThrottleInput = CachedChaosVehicle->GetThrottleInput();
	CurrentState.BrakeInput = CachedChaosVehicle->GetBrakeInput();
	CurrentState.SteeringInput = CachedChaosVehicle->GetSteeringInput();
	CurrentState.CurrentGear = CachedChaosVehicle->GetCurrentGear();
	CurrentState.bEngineRunning = true; // Chaos Vehicle engine is always running when active
	CurrentState.EngineLoad = FMath::Clamp(CurrentState.ThrottleInput * 0.7f + (CurrentState.RPM / 7000.0f) * 0.3f, 0.0f, 1.0f);
	CurrentState.PowertrainType = PowertrainType;

	// tyre noise follows how hard the tyres are sliding, not how fast the car is going. Take the
	// worst wheel: one locked wheel is audible even if the other three are gripping
	float WorstSlip = 0.0f;
	bool bAnySkidding = false;

	// the surface under the wheel that is sliding hardest is the one being heard
	float SlipOfSurfaceWheel = -1.0f;
	ETireSurfaceType DetectedSurface = ETireSurfaceType::Asphalt;

	const int32 NumWheels = CachedChaosVehicle->Wheels.Num();

	for (int32 WheelIndex = 0; WheelIndex < NumWheels; ++WheelIndex)
	{
		const FWheelStatus& Wheel = CachedChaosVehicle->GetWheelState(WheelIndex);

		if (!Wheel.bInContact)
		{
			continue;
		}

		// both magnitudes are signed: a wheel dragging under braking reports the negative of one
		// spinning up under power. Taking the larger of the raw values kept only wheelspin and
		// threw every braking and cornering slide away, so the tyres were silent exactly when they
		// should have been loudest. A slide is a slide whichever way the wheel is going wrong
		const float WheelSlip = FMath::Max(FMath::Abs(Wheel.SlipMagnitude), FMath::Abs(Wheel.SkidMagnitude));

		WorstSlip = FMath::Max(WorstSlip, WheelSlip);
		bAnySkidding |= Wheel.bIsSkidding;

		if (WheelSlip > SlipOfSurfaceWheel)
		{
			SlipOfSurfaceWheel = WheelSlip;

			if (const UPhysicalMaterial* Material = Wheel.PhysMaterial.Get())
			{
				if (const ETireSurfaceType* Mapped = SurfaceTypeMapping.Find(Material->SurfaceType))
				{
					DetectedSurface = *Mapped;
				}
			}
		}
	}

	CurrentState.TireSurface = DetectedSurface;

	// measured from the threshold rather than from zero, so ordinary cornering stays silent and
	// the range that is audible is spent on actual sliding
	const float SlipRange = FMath::Max(TireSlipReference - TireSlipThreshold, 1.0f);
	CurrentState.TireSlip = FMath::Clamp((WorstSlip - TireSlipThreshold) / SlipRange, 0.0f, 1.0f);

	// takes more slip to start sliding than to keep sliding, so a car held on the limit settles on
	// an answer instead of alternating
	CurrentState.bTireSliding = CurrentState.bTireSliding
		? WorstSlip > TireSlipThreshold * TireSlipReleaseRatio
		: WorstSlip > TireSlipThreshold;

	if (SlipOverride >= 0.0f)
	{
		CurrentState.TireSlip = FMath::Clamp(SlipOverride, 0.0f, 1.0f);
		CurrentState.bTireSliding = CurrentState.TireSlip > KINDA_SMALL_NUMBER;
	}
	CurrentState.bTireSkidding = bAnySkidding;
}

void UVehicleSoundComponent::UpdateDynamicLayers(float DeltaTime)
{
	for (UDynamicSoundLayer* Layer : DynamicLayers)
	{
		if (Layer && Layer->IsActive())
		{
			// a source that ended by itself leaves the layer active but silent, and nothing else
			// in the update path notices
			Layer->RestartIfStopped();
			Layer->Update(DeltaTime, CurrentState);
		}
	}
}

// --- State Setters ---

void UVehicleSoundComponent::SetVehicleSoundState(const FVehicleSoundState& InState)
{
	CurrentState = InState;
}

void UVehicleSoundComponent::SetEngineRunning(bool bRunning)
{
	CurrentState.bEngineRunning = bRunning;
	// Activate/deactivate engine layers based on engine state
	for (UDynamicSoundLayer* Layer : DynamicLayers)
	{
		if (!Layer) continue;
		EDynamicSoundLayerType Type = Layer->GetLayerType();
		if (Type == EDynamicSoundLayerType::Engine || Type == EDynamicSoundLayerType::EVMotor || Type == EDynamicSoundLayerType::Exhaust)
		{
			if (bRunning)
			{
				Layer->Activate();
			}
			else
			{
				Layer->Deactivate();
			}
		}
	}
}

void UVehicleSoundComponent::SetCurrentRPM(float InRPM) { CurrentState.RPM = InRPM; }
void UVehicleSoundComponent::SetCurrentSpeed(float InSpeedKmh) { CurrentState.Speed = InSpeedKmh; }
void UVehicleSoundComponent::SetThrottleInput(float InThrottle) { CurrentState.ThrottleInput = FMath::Clamp(InThrottle, 0.0f, 1.0f); }
void UVehicleSoundComponent::SetBrakeInput(float InBrake) { CurrentState.BrakeInput = FMath::Clamp(InBrake, 0.0f, 1.0f); }
void UVehicleSoundComponent::SetSteeringInput(float InSteering) { CurrentState.SteeringInput = FMath::Clamp(InSteering, -1.0f, 1.0f); }
void UVehicleSoundComponent::SetCurrentGear(int32 InGear) { CurrentState.CurrentGear = InGear; }
void UVehicleSoundComponent::SetTireSurface(ETireSurfaceType InSurface) { CurrentState.TireSurface = InSurface; }
void UVehicleSoundComponent::SetWindowOpenAmount(float InAmount) { CurrentState.WindowOpenAmount = FMath::Clamp(InAmount, 0.0f, 1.0f); }

// --- Interaction ---

void UVehicleSoundComponent::PlayInteractionSound(EInteractionSoundType Type)
{
	if (InteractionHandler)
	{
		InteractionHandler->PlaySound(Type);
	}
}

void UVehicleSoundComponent::ReportImpact(const FVector& Location, float ImpactSpeed)
{
	if (ImpactHandler)
	{
		ImpactHandler->ReportImpact(Location, ImpactSpeed);
	}
}

void UVehicleSoundComponent::StartTurnSignal(float TickInterval)
{
	if (InteractionHandler)
	{
		InteractionHandler->PlaySound(EInteractionSoundType::TurnSignalOn);
		InteractionHandler->StartRepeatingSound(EInteractionSoundType::TurnSignalTick, TickInterval);
	}
}

void UVehicleSoundComponent::StopTurnSignal()
{
	if (InteractionHandler)
	{
		InteractionHandler->StopRepeatingSound(EInteractionSoundType::TurnSignalTick);
		InteractionHandler->PlaySound(EInteractionSoundType::TurnSignalOff);
	}
}

// --- Infotainment ---

void UVehicleSoundComponent::PlayTouchFeedback()
{
	if (InfotainmentHandler)
	{
		InfotainmentHandler->PlayTouchFeedback();
	}
}

void UVehicleSoundComponent::PlayNotification()
{
	if (InfotainmentHandler)
	{
		InfotainmentHandler->PlayNotification();
	}
}

// --- Volume ---

void UVehicleSoundComponent::SetLayerVolume(EDynamicSoundLayerType LayerType, float InVolume)
{
	for (UDynamicSoundLayer* Layer : DynamicLayers)
	{
		if (Layer && Layer->GetLayerType() == LayerType)
		{
			Layer->SetVolume(InVolume);
		}
	}
}

void UVehicleSoundComponent::LogSoundState() const
{
	const AActor* Owner = GetOwner();

	UE_LOG(LogVehicleSoundSystem, Display,
		TEXT("%s  %.0f km/h  %.0f rpm  slip %.2f  skidding %d  inside %d  gear %d"),
		Owner ? *Owner->GetName() : TEXT("?"), CurrentState.Speed, CurrentState.RPM,
		CurrentState.TireSlip, CurrentState.bTireSkidding ? 1 : 0,
		CurrentState.bListenerInside ? 1 : 0, CurrentState.CurrentGear);

	for (const UDynamicSoundLayer* Layer : DynamicLayers)
	{
		if (!Layer)
		{
			continue;
		}

		const UAudioComponent* Audio = Layer->GetAudioComponent();

		// the layer's own volume is only the ceiling; what reaches the mix is what the layer
		// worked out this frame from speed, revs and slip, and that is the number worth seeing
		UE_LOG(LogVehicleSoundSystem, Display,
			TEXT("    %-13s active %d  playing %d  layerVol %.2f  heard %.2f  sound %s"),
			*UEnum::GetDisplayValueAsText(Layer->GetLayerType()).ToString(),
			Layer->IsActive() ? 1 : 0, (Audio && Audio->IsPlaying()) ? 1 : 0,
			Layer->GetVolume(), Audio ? Audio->VolumeMultiplier : 0.0f,
			(Audio && Audio->Sound) ? *Audio->Sound->GetName() : TEXT("(none)"));
	}
}

void UVehicleSoundComponent::SetCategoryVolume(EVehicleSoundCategory Category, float InVolume)
{
	float ClampedVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);

	switch (Category)
	{
	case EVehicleSoundCategory::Dynamic:
		for (UDynamicSoundLayer* Layer : DynamicLayers)
		{
			if (Layer) Layer->SetVolume(ClampedVolume);
		}
		break;
	case EVehicleSoundCategory::Interaction:
		if (InteractionHandler) InteractionHandler->SetVolumeMultiplier(ClampedVolume);
		// collisions are an interaction sound too, and were the one handler this never reached,
		// which left crash volume with no runtime control at all
		if (ImpactHandler) ImpactHandler->SetVolumeMultiplier(ClampedVolume);
		break;
	case EVehicleSoundCategory::Infotainment:
		if (InfotainmentHandler) InfotainmentHandler->SetVolumeMultiplier(ClampedVolume);
		if (MusicPlayer) MusicPlayer->SetVolume(ClampedVolume);
		break;
	}
}
