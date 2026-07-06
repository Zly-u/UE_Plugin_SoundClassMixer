#pragma once

#include "Curves/CurveFloat.h"
#include "Sound/SoundSourceBusSend.h"

#include "SoundClassMixerSourceBusSendInfo.generated.h"

class USoundSourceBus;
class UAudioBus;


/**
 * Blueprint-friendly mirror of FSoundSourceBusSendInfo.
 *
 * The engine's FSoundSourceBusSendInfo (4.27) is missing BlueprintReadWrite on
 * SourceBusSendLevelControlMethod, SoundSourceBus, AudioBus and SendLevel, so
 * those fields are not exposed as pins in Blueprint. This struct re-declares
 * every field with BlueprintReadWrite and provides a conversion to the engine
 * struct for use with FActiveSound::SetSourceBusSend.
 */
USTRUCT(BlueprintType)
struct SOUNDCLASSMIXER_API FSoundClassMixerSourceBusSendInfo
{
	GENERATED_USTRUCT_BODY()

	/*
		Manual: Use Send Level only
		Linear: Interpolate between Min and Max Send Levels based on listener distance (between Distance Min and Distance Max)
		Custom Curve: Use the float curve to map Send Level to distance (0.0-1.0 on curve maps to Distance Min - Distance Max)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	ESourceBusSendLevelControlMethod SourceBusSendLevelControlMethod;

	// A source Bus to send the audio to. Source buses sonify (make audible) the audio sent to it and are themselves sounds which take up a voice slot in the audio engine.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	USoundSourceBus* SoundSourceBus;

	// An audio bus to send the audio to. Audio buses can be used to route audio to DSP effects or other purposes. E.g. side-chaining, analysis, etc. Audio buses are not audible unless hooked up to a source bus.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	UAudioBus* AudioBus;

	// The amount of audio to send to the bus.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float SendLevel;

	// The amount to send to the bus when sound is located at a distance equal to value specified in the min send distance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float MinSendLevel;

	// The amount to send to the bus when sound is located at a distance equal to value specified in the max send distance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float MaxSendLevel;

	// The distance at which the min send Level is sent to the bus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float MinSendDistance;

	// The distance at which the max send level is sent to the bus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float MaxSendDistance;

	// The custom curve to use for distance-based bus send level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	FRuntimeFloatCurve CustomSendLevelCurve;

	FSoundClassMixerSourceBusSendInfo()
		: SourceBusSendLevelControlMethod(ESourceBusSendLevelControlMethod::Linear)
		, SoundSourceBus(nullptr)
		, AudioBus(nullptr)
		, SendLevel(1.0f)
		, MinSendLevel(1.0f) // flipped because was requested by John R1se
		, MaxSendLevel(0.0f) // flipped because was requested by John R1se
		, MinSendDistance(100.0f)
		, MaxSendDistance(1000.0f)
	{
	}

	/** Converts to the engine struct for use with FActiveSound::SetSourceBusSend. */
	FSoundSourceBusSendInfo ToEngineStruct() const
	{
		FSoundSourceBusSendInfo Out;
		Out.SourceBusSendLevelControlMethod = SourceBusSendLevelControlMethod;
		Out.SoundSourceBus = SoundSourceBus;
		Out.AudioBus = AudioBus;
		Out.SendLevel = SendLevel;
		Out.MinSendLevel = MinSendLevel;
		Out.MaxSendLevel = MaxSendLevel;
		Out.MinSendDistance = MinSendDistance;
		Out.MaxSendDistance = MaxSendDistance;
		Out.CustomSendLevelCurve = CustomSendLevelCurve;
		return Out;
	}
};