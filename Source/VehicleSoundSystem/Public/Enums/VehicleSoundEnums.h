#pragma once

#include "CoreMinimal.h"
#include "VehicleSoundEnums.generated.h"

/** Vehicle powertrain type - determines which sound layers to activate */
UENUM(BlueprintType)
enum class EVehiclePowertrainType : uint8
{
	ICE		UMETA(DisplayName = "Internal Combustion Engine"),
	EV		UMETA(DisplayName = "Electric Vehicle"),
	Hybrid	UMETA(DisplayName = "Hybrid (Reserved)")
};

/** Top-level sound category for volume/mute control */
UENUM(BlueprintType)
enum class EVehicleSoundCategory : uint8
{
	Dynamic			UMETA(DisplayName = "Dynamic (Driving)"),
	Interaction		UMETA(DisplayName = "Interaction"),
	Infotainment	UMETA(DisplayName = "Infotainment")
};

/** Dynamic sound layer type */
UENUM(BlueprintType)
enum class EDynamicSoundLayerType : uint8
{
	Engine,
	EVMotor,
	Exhaust,
	Tire,
	Wind,
	Transmission
};

/** Tire surface type for surface-dependent tire sounds */
UENUM(BlueprintType)
enum class ETireSurfaceType : uint8
{
	Asphalt,
	Concrete,
	Gravel,
	Dirt,
	Wet,
	Snow
};

/** Interaction sound types for one-shot playback */
UENUM(BlueprintType)
enum class EInteractionSoundType : uint8
{
	DoorOpen,
	DoorClose,
	ButtonPress,
	SeatbeltBuckle,
	SeatbeltUnbuckle,
	TurnSignalOn,
	TurnSignalOff,
	TurnSignalTick,
	Honk,
	WindowUp,
	WindowDown,
	GearShift,
	HandbrakeEngage,
	HandbrakeRelease,
	KeyIgnition,
	WiperOn,
	WiperOff
};

/** Bundles all vehicle state needed for audio updates */
USTRUCT(BlueprintType)
struct VEHICLESOUNDSYSTEM_API FVehicleSoundState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound")
	float RPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound")
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ThrottleInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BrakeInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float SteeringInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound")
	int32 CurrentGear = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EngineLoad = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound")
	bool bEngineRunning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound")
	ETireSurfaceType TireSurface = ETireSurfaceType::Asphalt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound")
	EVehiclePowertrainType PowertrainType = EVehiclePowertrainType::ICE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindowOpenAmount = 0.0f;
};
