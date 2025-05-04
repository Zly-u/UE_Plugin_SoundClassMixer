#pragma once

#include "Tickable.h"
#include "DSP/VolumeFader.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SoundClassMixerSubsystem.generated.h"


class USoundClass;
class USoundClassMixerBlueprintFunctionLibrary;
class FSoundClassMixerCommands;
enum class EAudioFaderCurve : uint8;


DECLARE_LOG_CATEGORY_CLASS(LogSoundClassMixerSubsystem, Display, All);


USTRUCT()
struct FSoundSubSysProperties
{
	GENERATED_BODY()
	
	UPROPERTY()
		bool bIsFading = false;
	
	Audio::FVolumeFader Fader;
};


/**
 * A Simple Sound Mixer Subsystem for USoundClass's.
 */
UCLASS()
class SOUNDCLASSMIXER_API USoundClassMixerSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	friend FSoundClassMixerCommands;
	friend USoundClassMixerBlueprintFunctionLibrary;

	
public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~USubsystem
	
	// FTickableGameObject
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsAllowedToTick() const override final;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// ~FTickableGameObject

	bool IsInitialized() const { return bInitialized; }

	
private:
	void GatherSoundClasses();
	
	void AdjustSoundClassVolumeInternal(
		const USoundClass*     SoundClassAsset, float AdjustVolumeDuration, float AdjustVolumeLevel, bool bInIsFadeOut,
		const EAudioFaderCurve FadeCurve
	);
	USoundClass*  FindSoundClassByName(const FString& SoundClassName);
	
	void AdjustSoundSubmixVolumeInternal(
		const USoundSubmix* SoundSubmixAsset, float AdjustVolumeDuration, float AdjustVolumeLevel, bool bInIsFadeOut,
		EAudioFaderCurve    FadeCurve
	);
	USoundSubmix* FindSoundSubmixByName(const FString& SoundSubmixName);
	
	void UpdateAudioClasses();

	
public:
	UPROPERTY()
		TMap<USoundClass*, FSoundSubSysProperties> SoundClassMap;
	
	UPROPERTY()
		TMap<USoundSubmix*, FSoundSubSysProperties> SoundSubmixMap;

	
private:
	bool bInitialized = false;
};
