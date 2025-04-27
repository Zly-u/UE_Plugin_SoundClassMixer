#pragma once

#include "Tickable.h"
#include "Debug/DebugDrawService.h"
#include "DSP/VolumeFader.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SoundClassMixerSubsystem.generated.h"


class USoundClass;
class USoundClassMixerBlueprintFunctionLibrary;
class FSoundClassMixerCommands;
enum class EAudioFaderCurve : uint8;


DECLARE_LOG_CATEGORY_CLASS(LogSoundClassMixerSubsystem, Display, All);


USTRUCT()
struct FSoundClassSubSysProperties
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
	void AdjustVolumeInternal(const USoundClass* SoundClassAsset, float AdjustVolumeDuration, float AdjustVolumeLevel, bool bInIsFadeOut, const EAudioFaderCurve FadeCurve);
	void UpdateAudioClasses();

	USoundClass* FindSoundClassByName(const FString& SoundClassName);

#if WITH_EDITOR
private:
	void OnSoundClassAssetAdded(const FAssetData& AssetData);
	void OnSoundClassAssetRemoved(const FAssetData& AssetData);
#endif

public:
	UPROPERTY()
		TMap<USoundClass*, FSoundClassSubSysProperties> SoundClassMap;
	
private:
	bool bInitialized = false;
};
