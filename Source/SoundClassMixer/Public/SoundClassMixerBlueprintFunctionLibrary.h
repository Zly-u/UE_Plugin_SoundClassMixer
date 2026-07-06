#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Sound/SoundSourceBusSend.h"
#include "SoundClassMixerSourceBusSendInfo.h"

#include "SoundClassMixerBlueprintFunctionLibrary.generated.h"

class USoundConcurrency;
class USoundClass;
class USoundSubmix;
class UAudioComponent;
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
		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = SoundClassMixerPlugin, meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext))
			static void SoundClassFadeTo(
				const UObject* WorldContextObject,
				USoundClass* TargetClass,
				const float FadeDuration, const float FadeVolumeLevel,
				const EAudioFaderCurve FadeCurve
			);

		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = SoundClassMixerPlugin, meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext))
			static void SetSoundClassVolume(const UObject* WorldContextObject, USoundClass* TargetClass, const float NewVolume);
		
		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = SoundClassMixerPlugin)
			static float GetSoundClassVolume(USoundClass* TargetClass);

		
	public:
		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = SoundClassMixerPlugin, meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext))
			static void SoundSubmixFadeTo(
				const UObject* WorldContextObject,
				USoundSubmix* TargetClass, float FadeDuration, float FadeVolumeLevel,
				EAudioFaderCurve FadeCurve
			);

		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = SoundClassMixerPlugin, meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext))
			static void SetSoundSubmixVolume(const UObject* WorldContextObject, USoundSubmix* TargetClass, float NewVolume);
		
		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = SoundClassMixerPlugin)
			static float GetSoundSubmixVolume(USoundSubmix* TargetClass);
		

	public:
		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SoundClassMixerPlugin|Audio Play", meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2", UnsafeDuringActorConstruction = "true"))
		static void PlaySound2D_WithSubmixOverride(
			const UObject* WorldContextObject,
			USoundBase* Sound,
			float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f,
			USoundConcurrency* ConcurrencySettings = nullptr,
			AActor* OwningActor = nullptr,
			bool bIsUISound = true,
			USoundSubmixBase* SubmixOverride = nullptr
		);
		
		UFUNCTION(BlueprintCallable, Category="SoundClassMixerPlugin|Audio Play", meta=(WorldContext="WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "3", UnsafeDuringActorConstruction = "true", Keywords = "play"))
		static void PlaySoundAtLocation_WithSubmixOverride(
			const UObject* WorldContextObject,
			USoundBase* Sound,
			FVector Location, FRotator Rotation,
			float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f,
			USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr,
			AActor* OwningActor = nullptr,
			USoundSubmixBase* SubmixOverride = nullptr
		);
		
		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SoundClassMixerPlugin|Audio Spawn", meta=(AdvancedDisplay = "2", UnsafeDuringActorConstruction = "true", Keywords = "play"))
		static UAudioComponent* SpawnSoundAttached_WithSubmixOverride(
			USoundBase* Sound,
			USceneComponent* AttachToComponent, FName AttachPointName = NAME_None,
			FVector Location = FVector(ForceInit), FRotator Rotation = FRotator::ZeroRotator,
			EAttachLocation::Type LocationType = EAttachLocation::KeepRelativeOffset,
			bool bStopWhenAttachedToDestroyed = false,
			float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f,
			USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr,
			bool bAutoDestroy = true,
			USoundSubmixBase* SubmixOverride = nullptr
		);

		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SoundClassMixerPlugin|Audio Spawn", meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "3", UnsafeDuringActorConstruction = "true", Keywords = "play"))
		static UAudioComponent* SpawnSoundAtLocation_WithSubmixOverride(
			const UObject* WorldContextObject,
			USoundBase* Sound,
			FVector Location, FRotator Rotation = FRotator::ZeroRotator,
			float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, float StartTime = 0.f,
			USoundAttenuation* AttenuationSettings = nullptr, USoundConcurrency* ConcurrencySettings = nullptr,
			bool bAutoDestroy = true,
			USoundSubmixBase* SubmixOverride = nullptr
		);
		
		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SoundClassMixerPlugin|Audio Spawn", meta=(WorldContext = "WorldContextObject", CallableWithoutWorldContext, AdvancedDisplay = "2", UnsafeDuringActorConstruction = "true", Keywords = "play"))
		static UAudioComponent* SpawnSound2D_WithSubmixOverride(
			const UObject* WorldContextObject,
			USoundBase* Sound,
			float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, float StartTime = 0.f,
			USoundConcurrency* ConcurrencySettings = nullptr,
			bool bPersistAcrossLevelTransition = false, bool bAutoDestroy = true,
			USoundSubmixBase* SubmixOverride = nullptr
		);
		
		
	public:
		/**
		 * Sets the Pre-Effect bus sends for a currently-playing AudioComponent.
		 * Each entry in BusSends is forwarded to FActiveSound::SetSourceBusSend on the
		 * audio thread, which overrides the SendLevel of an existing matching bus send
		 * (matched by SoundSourceBus OR AudioBus pointer) or appends a new one if no
		 * match is found. An entry may target either a source bus (set SoundSourceBus)
		 * or an audio bus (set AudioBus) — the engine's SetBusSendffectInternal carried
		 * both kinds in one call; here they're unified through the array entry's fields.
		 *
		 * Note: only SendLevel is merged on a match; other fields (control method,
		 * min/max levels, distance range, custom curve) on an already-present bus send
		 * are NOT updated by the engine's SetSourceBusSend. For a fresh entry, the
		 * full FSoundSourceBusSendInfo is applied.
		 *
		 * Also note: the source sound asset must have bEnableBusSends = true for bus
		 * sends to actually be processed by the mixer source.
		 */
		UFUNCTION(BlueprintCallable, Category = "SoundClassMixerPlugin|BusSends")
		static void SetAudioBusSendsPreEffect(
			UAudioComponent* AudioComponent,
			const TArray<FSoundClassMixerSourceBusSendInfo>& BusSends
		);

		/**
		 * Sets the Post-Effect bus sends for a currently-playing AudioComponent.
		 * See SetSourceBusSendsPreEffect for semantics; this targets EBusSendType::PostEffect.
		 */
		UFUNCTION(BlueprintCallable, Category = "SoundClassMixerPlugin|BusSends")
		static void SetAudioBusSendsPostEffect(
			UAudioComponent* AudioComponent,
			const TArray<FSoundClassMixerSourceBusSendInfo>& BusSends
		);


	public:
		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SoundClassMixerPlugin|Utils")
			static float ConvertToLinear(const float Value);

		UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SoundClassMixerPlugin|Utils")
			static float ConvertToDecibels(const float Value);
};
