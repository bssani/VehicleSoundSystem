#include "Components/VehicleSoundComponent.h"
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

	UpdateDynamicLayers(DeltaTime);
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
}

void UVehicleSoundComponent::UpdateDynamicLayers(float DeltaTime)
{
	for (UDynamicSoundLayer* Layer : DynamicLayers)
	{
		if (Layer && Layer->IsActive())
		{
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
		break;
	case EVehicleSoundCategory::Infotainment:
		if (InfotainmentHandler) InfotainmentHandler->SetVolumeMultiplier(ClampedVolume);
		if (MusicPlayer) MusicPlayer->SetVolume(ClampedVolume);
		break;
	}
}
