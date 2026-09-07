#include "Subsystem/VehicleSoundSubsystem.h"
#include "Components/VehicleSoundComponent.h"
#include "VehicleSoundSystemModule.h"
#include "Settings/VehicleSoundSettings.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "DynamicSound/DynamicSoundLayer.h"

void UVehicleSoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UVehicleSoundSettings* Settings = GetDefault<UVehicleSoundSettings>();

	ReducedDistance = Settings->ReducedDistance;
	EngineOnlyDistance = Settings->EngineOnlyDistance;
	CullDistance = Settings->CullDistance;

	MasterVolume = Settings->MasterVolume;
	CategoryVolumes.Add(EVehicleSoundCategory::Dynamic, Settings->DynamicVolume);
	CategoryVolumes.Add(EVehicleSoundCategory::Interaction, Settings->InteractionVolume);
	CategoryVolumes.Add(EVehicleSoundCategory::Infotainment, Settings->InfotainmentVolume);

	UE_LOG(LogVehicleSoundSystem, Log,
		TEXT("VehicleSoundSubsystem initialized. LOD reduced/engine-only/cull: %.0f / %.0f / %.0f cm."),
		ReducedDistance, EngineOnlyDistance, CullDistance);
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

// --- Console commands ---
//
// Working out which layer is responsible for a noise is otherwise guesswork: every layer plays
// into the same mix on the same actor, and a tyre graph and an engine graph can sound alike. These
// silence one layer at a time across every vehicle, which answers it in one go.

namespace
{
	bool ParseLayerType(const FString& Name, EDynamicSoundLayerType& OutType)
	{
		static const TMap<FString, EDynamicSoundLayerType> Names =
		{
			{ TEXT("engine"),       EDynamicSoundLayerType::Engine },
			{ TEXT("evmotor"),      EDynamicSoundLayerType::EVMotor },
			{ TEXT("exhaust"),      EDynamicSoundLayerType::Exhaust },
			{ TEXT("tire"),         EDynamicSoundLayerType::Tire },
			{ TEXT("wind"),         EDynamicSoundLayerType::Wind },
			{ TEXT("transmission"), EDynamicSoundLayerType::Transmission }
		};

		if (const EDynamicSoundLayerType* Found = Names.Find(Name.ToLower()))
		{
			OutType = *Found;
			return true;
		}

		return false;
	}

	UVehicleSoundSubsystem* GetSubsystem(const UWorld* World)
	{
		return (World && World->GetGameInstance())
			? World->GetGameInstance()->GetSubsystem<UVehicleSoundSubsystem>()
			: nullptr;
	}
}

static FAutoConsoleCommandWithWorldAndArgs GVehicleSoundLayerVolume(
	TEXT("vs.LayerVolume"),
	TEXT("vs.LayerVolume <engine|evmotor|exhaust|tire|wind|transmission> <0..1> - sets that layer's volume on every vehicle. Silence one at a time to find which is making a noise."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic([](const TArray<FString>& Args, UWorld* World)
	{
		EDynamicSoundLayerType LayerType;

		if (Args.Num() < 2 || !ParseLayerType(Args[0], LayerType))
		{
			UE_LOG(LogVehicleSoundSystem, Warning, TEXT("Usage: vs.LayerVolume <engine|evmotor|exhaust|tire|wind|transmission> <0..1>"));
			return;
		}

		UVehicleSoundSubsystem* Subsystem = GetSubsystem(World);

		if (!Subsystem)
		{
			return;
		}

		const float Volume = FMath::Clamp(FCString::Atof(*Args[1]), 0.0f, 1.0f);
		int32 Count = 0;

		for (const TWeakObjectPtr<UVehicleSoundComponent>& Weak : Subsystem->GetActiveVehicles())
		{
			if (UVehicleSoundComponent* Component = Weak.Get())
			{
				Component->SetLayerVolume(LayerType, Volume);
				++Count;
			}
		}

		UE_LOG(LogVehicleSoundSystem, Display, TEXT("vs.LayerVolume: layer %s set to %.2f on %d vehicles."),
			*Args[0], Volume, Count);
	}));

static FAutoConsoleCommandWithWorld GVehicleSoundDumpState(
	TEXT("vs.DumpState"),
	TEXT("Writes every vehicle's speed, revs, slip and per-layer volumes to the log."),
	FConsoleCommandWithWorldDelegate::CreateStatic([](UWorld* World)
	{
		UVehicleSoundSubsystem* Subsystem = GetSubsystem(World);

		if (!Subsystem)
		{
			return;
		}

		for (const TWeakObjectPtr<UVehicleSoundComponent>& Weak : Subsystem->GetActiveVehicles())
		{
			if (const UVehicleSoundComponent* Component = Weak.Get())
			{
				Component->LogSoundState();
			}
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs GVehicleSoundForceSlip(
	TEXT("vs.ForceSlip"),
	TEXT("vs.ForceSlip <0..1 | -1> - pins tyre slip on every vehicle, or -1 to hand it back to the wheels. Pin it to 1 to hear whether the tyre graph reacts to slip at all."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic([](const TArray<FString>& Args, UWorld* World)
	{
		UVehicleSoundSubsystem* Subsystem = GetSubsystem(World);

		if (Args.Num() < 1 || !Subsystem)
		{
			UE_LOG(LogVehicleSoundSystem, Warning, TEXT("Usage: vs.ForceSlip <0..1 | -1>"));
			return;
		}

		const float Value = FCString::Atof(*Args[0]);
		int32 Count = 0;

		for (const TWeakObjectPtr<UVehicleSoundComponent>& Weak : Subsystem->GetActiveVehicles())
		{
			if (UVehicleSoundComponent* Component = Weak.Get())
			{
				Component->SlipOverride = Value;
				++Count;
			}
		}

		UE_LOG(LogVehicleSoundSystem, Display, TEXT("vs.ForceSlip: slip %s on %d vehicles."),
			Value < 0.0f ? TEXT("released") : *FString::Printf(TEXT("pinned to %.2f"), Value), Count);
	}));

static FAutoConsoleCommandWithWorldAndArgs GVehicleSoundSlipThreshold(
	TEXT("vs.SlipThreshold"),
	TEXT("vs.SlipThreshold <fraction> [reference] - slip as a fraction of road speed that counts as sliding, on every vehicle. Ordinary cornering measures about 0.03, a real slide about 0.27. Takes effect immediately, unlike editing the Blueprint."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic([](const TArray<FString>& Args, UWorld* World)
	{
		UVehicleSoundSubsystem* Subsystem = GetSubsystem(World);

		if (Args.Num() < 1 || !Subsystem)
		{
			UE_LOG(LogVehicleSoundSystem, Warning, TEXT("Usage: vs.SlipThreshold <fraction> [reference]  (cornering ~0.03, sliding ~0.27)"));
			return;
		}

		const float Threshold = FMath::Max(FCString::Atof(*Args[0]), 0.0f);
		const float Reference = Args.Num() > 1 ? FCString::Atof(*Args[1]) : -1.0f;
		int32 Count = 0;

		for (const TWeakObjectPtr<UVehicleSoundComponent>& Weak : Subsystem->GetActiveVehicles())
		{
			if (UVehicleSoundComponent* Component = Weak.Get())
			{
				Component->TireSlipThreshold = Threshold;

				if (Reference > 0.0f)
				{
					Component->TireSlipReference = Reference;
				}

				++Count;
			}
		}

		UE_LOG(LogVehicleSoundSystem, Display, TEXT("vs.SlipThreshold: threshold %.3f%s on %d vehicles."),
			Threshold, Reference > 0.0f ? *FString::Printf(TEXT(", reference %.3f"), Reference) : TEXT(""), Count);
	}));
