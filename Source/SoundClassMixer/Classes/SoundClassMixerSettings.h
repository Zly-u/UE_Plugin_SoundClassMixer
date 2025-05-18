#pragma once

#include "UObject/Object.h"

#include "SoundClassMixerSettings.generated.h"

/**
 * Settings for SoundClassMixer Subsystem.
 */
UCLASS(Config = "SoundClassMixer", DefaultConfig, meta = (DisplayName = "Sound Class Mixer Settings"))
class SOUNDCLASSMIXER_API USoundClassMixerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	USoundClassMixerSettings();
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Filtering")
		TArray<FString> ExcludedSoundClassNames = {};
	
	UPROPERTY(Config, EditAnywhere, Category = "Filtering")
		TArray<TSoftObjectPtr<USoundClass>> ExcludedSoundClasses;
};
