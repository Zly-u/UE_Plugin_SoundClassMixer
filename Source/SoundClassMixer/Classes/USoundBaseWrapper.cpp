#include "USoundBaseWrapper.h"

#include "Audio.h"
#include "Sound/SoundSubmix.h"
#include "Sound/SoundSubmixSend.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundAttenuation.h"


void USoundBaseWrapper::InheritSoundBase(USoundBase* OriginalSoundBase)
{
	InnerSound = OriginalSoundBase;
	SoundSubmixSends = InnerSound->SoundSubmixSends;
}


USoundSubmixBase* USoundBaseWrapper::GetSoundSubmix() const
{
	if (SoundSubmixOverride)
	{
		return SoundSubmixOverride;
	}
	return InnerSound ? InnerSound->GetSoundSubmix() : Super::GetSoundSubmix();
}


#pragma region ForwardingOverrides // ==================================================================================


bool USoundBaseWrapper::IsPlayable() const
{
	return InnerSound ? InnerSound->IsPlayable() : Super::IsPlayable();
}

void USoundBaseWrapper::Parse(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances)
{
	if (InnerSound)
	{
		InnerSound->Parse(AudioDevice, NodeWaveInstanceHash, ActiveSound, ParseParams, WaveInstances);
	}
}

USoundClass* USoundBaseWrapper::GetSoundClass() const
{
	return InnerSound ? InnerSound->GetSoundClass() : Super::GetSoundClass();
}

float USoundBaseWrapper::GetDuration()
{
	return InnerSound ? InnerSound->GetDuration() : Super::GetDuration();
}

float USoundBaseWrapper::GetMaxDistance() const
{
	return InnerSound ? InnerSound->GetMaxDistance() : Super::GetMaxDistance();
}

float USoundBaseWrapper::GetVolumeMultiplier()
{
	return InnerSound ? InnerSound->GetVolumeMultiplier() : Super::GetVolumeMultiplier();
}

float USoundBaseWrapper::GetPitchMultiplier()
{
	return InnerSound ? InnerSound->GetPitchMultiplier() : Super::GetPitchMultiplier();
}

const FSoundAttenuationSettings* USoundBaseWrapper::GetAttenuationSettingsToApply() const
{
	return InnerSound ? InnerSound->GetAttenuationSettingsToApply() : Super::GetAttenuationSettingsToApply();
}

bool USoundBaseWrapper::SupportsSubtitles() const
{
	return InnerSound ? InnerSound->SupportsSubtitles() : Super::SupportsSubtitles();
}

bool USoundBaseWrapper::HasAttenuationNode() const
{
	return InnerSound ? InnerSound->HasAttenuationNode() : Super::HasAttenuationNode();
}

bool USoundBaseWrapper::ShouldApplyInteriorVolumes()
{
	return InnerSound ? InnerSound->ShouldApplyInteriorVolumes() : Super::ShouldApplyInteriorVolumes();
}

bool USoundBaseWrapper::IsPlayWhenSilent() const
{
	return InnerSound ? InnerSound->IsPlayWhenSilent() : Super::IsPlayWhenSilent();
}

float USoundBaseWrapper::GetSubtitlePriority() const
{
	return InnerSound ? InnerSound->GetSubtitlePriority() : Super::GetSubtitlePriority();
}

bool USoundBaseWrapper::GetSoundWavesWithCookedAnalysisData(TArray<USoundWave*>& OutSoundWaves)
{
	return InnerSound ? InnerSound->GetSoundWavesWithCookedAnalysisData(OutSoundWaves) : Super::GetSoundWavesWithCookedAnalysisData(OutSoundWaves);
}

bool USoundBaseWrapper::HasCookedFFTData() const
{
	return InnerSound ? InnerSound->HasCookedFFTData() : Super::HasCookedFFTData();
}

bool USoundBaseWrapper::HasCookedAmplitudeEnvelopeData() const
{
	return InnerSound ? InnerSound->HasCookedAmplitudeEnvelopeData() : Super::HasCookedAmplitudeEnvelopeData();
}

#pragma endregion ForwardingOverrides // ===============================================================================