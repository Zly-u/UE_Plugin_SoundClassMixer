#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SoundClassMixerBlueprintFunctionLibrary.generated.h"

class USoundClass;
enum class EAudioFaderCurve : uint8;

DECLARE_LOG_CATEGORY_CLASS(LogSoundClassMixer, Display, All)

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
			const float FadeOutDuration, const float FadeVolumeLevel,
			const EAudioFaderCurve FadeCurve
		);

	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin)
		static void SetSoundClassVolume(USoundClass* TargetClass, float NewVolume);
	
	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin)
		static float GetSoundClassVolume(USoundClass* TargetClass);

	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin)
		static float ConvertToLinear(const float Value);

	UFUNCTION(BlueprintCallable, Category = SoundClassMixerPlugin)
		static float ConvertToDecibels(const float Value);
};
