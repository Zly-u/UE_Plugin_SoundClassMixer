#include "SoundClassMixer.h"

#define LOCTEXT_NAMESPACE "SoundClassMixerModule"

void FSoundClassMixerModule::StartupModule()
{
	UE_LOG(LogSoundClassMixerModule, Verbose, TEXT("SoundClassMixerModule Loaded."));
}

void FSoundClassMixerModule::ShutdownModule()
{
	UE_LOG(LogSoundClassMixerModule, Verbose, TEXT("SoundClassMixerModule Unloaded."));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSoundClassMixerModule, SoundClassMixer)