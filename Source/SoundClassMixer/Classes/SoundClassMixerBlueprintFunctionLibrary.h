﻿#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SoundClassMixerBlueprintFunctionLibrary.generated.h"

class USoundClass;
class USoundSubmix;
enum class EAudioFaderCurve : uint8;


DECLARE_LOG_CATEGORY_CLASS(LogSoundClassMixer, Display, All)


UENUM(BlueprintType)
enum class EAudioFadeDirection : uint8
{
	In,
	Out
};


/**
 * Useful BP functions.
 */
UCLASS()
class SOUNDCLASSMIXER_API USoundClassMixerBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin, meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static void SoundClassFadeTo(
			const UObject* WorldContextObject,
			USoundClass* TargetClass,
			const float FadeDuration, const float FadeVolumeLevel,
			const EAudioFaderCurve FadeCurve
		);

	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin, meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static void SetSoundClassVolume(const UObject* WorldContextObject, USoundClass* TargetClass, const float NewVolume);
	
	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin)
		static float GetSoundClassVolume(USoundClass* TargetClass);

	
public:
	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin, meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static void SoundSubmixFadeTo(
			const UObject* WorldContextObject,
			USoundSubmix* TargetClass, float FadeDuration, float FadeVolumeLevel,
			EAudioFaderCurve FadeCurve
		);

	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin, meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext))
		static void SetSoundSubmixVolume(const UObject* WorldContextObject, USoundSubmix* TargetClass, float NewVolume);
	
	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin)
		static float GetSoundSubmixVolume(USoundSubmix* TargetClass);
	
public:
	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin)
		static float ConvertToLinear(const float Value);

	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin)
		static float ConvertToDecibels(const float Value);
};
