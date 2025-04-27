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
		TArray<FString> ExcludedSoundClassNames = {
			// Default list of Engine's built in Sound Classes.
			"Master",
			"Music",
			"Normal_Attack",
			"SFX",
			"Special_Attack_PSM",
			"Voice",
		};
	
	UPROPERTY(Config, EditAnywhere, Category = "Filtering")
		TArray<TSoftObjectPtr<USoundClass>> ExcludedSoundClasses;
};
