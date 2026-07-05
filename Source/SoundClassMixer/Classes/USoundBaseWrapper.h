#pragma once

#include "Sound/SoundBase.h"
#include "USoundBaseWrapper.generated.h"


/**
 * SoundBase wrapper that forwards all playback behavior to an inner USoundBase
 * while overriding the base submix routing.
 */
UCLASS()
class SOUNDCLASSMIXER_API USoundBaseWrapper : public USoundBase
{
	GENERATED_BODY()

	public:
		/** The original sound whose playback behavior this wrapper forwards. */
		UPROPERTY(Transient)
		USoundBase* InnerSound = nullptr;

		/** When set, returned by GetSoundSubmix() instead of the inner sound's submix. */
		UPROPERTY(EditAnywhere)
		USoundSubmixBase* SoundSubmixOverride = nullptr;

		
	public:
		void InheritSoundBase(USoundBase* OriginalSoundBase);

		
	public:
		virtual USoundSubmixBase* GetSoundSubmix() const override;
		
		
	public:
		//~ Begin USoundBase Forwarding overrides
		virtual bool IsPlayable() const override;
		virtual void Parse(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances) override;
		virtual USoundClass* GetSoundClass() const override;
		virtual float GetDuration() override;
		virtual float GetMaxDistance() const override;
		virtual float GetVolumeMultiplier() override;
		virtual float GetPitchMultiplier() override;
		virtual const FSoundAttenuationSettings* GetAttenuationSettingsToApply() const override;
		virtual bool SupportsSubtitles() const override;
		virtual bool HasAttenuationNode() const override;
		virtual bool ShouldApplyInteriorVolumes() override;
		virtual bool IsPlayWhenSilent() const override;
		virtual float GetSubtitlePriority() const override;
		virtual bool GetSoundWavesWithCookedAnalysisData(TArray<USoundWave*>& OutSoundWaves) override;
		virtual bool HasCookedFFTData() const override;
		virtual bool HasCookedAmplitudeEnvelopeData() const override;
		//~ End USoundBase Forwarding overrides
};