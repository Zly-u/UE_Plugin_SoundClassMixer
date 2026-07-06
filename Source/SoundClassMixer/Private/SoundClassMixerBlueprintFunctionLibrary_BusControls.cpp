#include "AudioDevice.h"
#include "SoundClassMixerBlueprintFunctionLibrary.h"
#include "Components/AudioComponent.h"


void USoundClassMixerBlueprintFunctionLibrary::SetAudioBusSendsPreEffect(
	UAudioComponent* AudioComponent,
	const TArray<FSoundClassMixerSourceBusSendInfo>& BusSends
)
{
	if (!AudioComponent)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("[SetSourceBusSendsPreEffect] AudioComponent was not set."));
		return;
	}

	if (FAudioDevice* AudioDevice = AudioComponent->GetAudioDevice())
	{
		const uint64 AudioComponentID = AudioComponent->GetAudioComponentID();

		// Convert Blueprint-friendly struct to engine struct.
		TArray<FSoundSourceBusSendInfo> EngineBusSends;
		EngineBusSends.Reserve(BusSends.Num());
		for (const FSoundClassMixerSourceBusSendInfo& Entry : BusSends)
		{
			EngineBusSends.Add(Entry.ToEngineStruct());
		}

		DECLARE_CYCLE_STAT(TEXT("FAudioThreadTask.SetSourceBusSendsPreEffect"), STAT_SetSourceBusSendsPreEffect, STATGROUP_AudioThreadCommands);
		FAudioThread::RunCommandOnAudioThread([AudioDevice, AudioComponentID, EngineBusSends = MoveTemp(EngineBusSends)]()
		{
			if (FActiveSound* ActiveSound = AudioDevice->FindActiveSound(AudioComponentID))
			{
				for (const FSoundSourceBusSendInfo& SendInfo : EngineBusSends)
				{
					ActiveSound->SetSourceBusSend(EBusSendType::PreEffect, SendInfo);
				}
			}
		}, GET_STATID(STAT_SetSourceBusSendsPreEffect));
	}
}

void USoundClassMixerBlueprintFunctionLibrary::SetAudioBusSendsPostEffect(
	UAudioComponent* AudioComponent,
	const TArray<FSoundClassMixerSourceBusSendInfo>& BusSends
)
{
	if (!AudioComponent)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("[SetSourceBusSendsPostEffect] AudioComponent was not set."));
		return;
	}

	if (FAudioDevice* AudioDevice = AudioComponent->GetAudioDevice())
	{
		const uint64 AudioComponentID = AudioComponent->GetAudioComponentID();

		TArray<FSoundSourceBusSendInfo> EngineBusSends;
		EngineBusSends.Reserve(BusSends.Num());
		for (const FSoundClassMixerSourceBusSendInfo& Entry : BusSends)
		{
			EngineBusSends.Add(Entry.ToEngineStruct());
		}

		DECLARE_CYCLE_STAT(TEXT("FAudioThreadTask.SetSourceBusSendsPostEffect"), STAT_SetSourceBusSendsPostEffect, STATGROUP_AudioThreadCommands);
		FAudioThread::RunCommandOnAudioThread([AudioDevice, AudioComponentID, EngineBusSends = MoveTemp(EngineBusSends)]()
		{
			if (FActiveSound* ActiveSound = AudioDevice->FindActiveSound(AudioComponentID))
			{
				for (const FSoundSourceBusSendInfo& SendInfo : EngineBusSends)
				{
					ActiveSound->SetSourceBusSend(EBusSendType::PostEffect, SendInfo);
				}
			}
		}, GET_STATID(STAT_SetSourceBusSendsPostEffect));
	}
}