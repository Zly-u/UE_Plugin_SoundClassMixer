#include "SoundClassMixerBlueprintFunctionLibrary.h"
#include "SoundClassMixerSubsystem.h"
#include "Sound/SoundSubmix.h"


void USoundClassMixerBlueprintFunctionLibrary::SoundSubmixFadeTo(
	const UObject* WorldContextObject,
	USoundSubmix* TargetClass,
	const float FadeDuration, const float FadeVolumeLevel,
	const EAudioFaderCurve FadeCurve
)
{
	if (!TargetClass)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("Could not find Sound Submix"));
		return;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	checkf(World, TEXT("World is invalid."))

	const UGameInstance* GI = World->GetGameInstance();
	checkf(GI, TEXT("GI is invalid."))
	
	USoundClassMixerSubsystem* SoundClassMixerSubsystem = GI->GetSubsystem<USoundClassMixerSubsystem>();
	checkf(SoundClassMixerSubsystem, TEXT("SoundClassMixerSubsystem is invalid."))

	checkf(SoundClassMixerSubsystem->SoundSubmixMap.Find(TargetClass), TEXT("SoundSubmix Properties are not found."))

	SoundClassMixerSubsystem->AdjustSoundSubmixVolumeInternal(
		TargetClass,
		FadeDuration, FadeVolumeLevel,
		TargetClass->OutputVolume > FadeVolumeLevel,
		FadeCurve
	);
}

void USoundClassMixerBlueprintFunctionLibrary::SetSoundSubmixVolume(const UObject* WorldContextObject, USoundSubmix* TargetClass, float NewVolume)
{
	if (!TargetClass)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("Could not find Sound Submix"));
		return;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	checkf(World, TEXT("World is invalid."))

	const UGameInstance* GI = World->GetGameInstance();
	checkf(GI, TEXT("GI is invalid."))
	
	USoundClassMixerSubsystem* SoundClassMixerSubsystem = GI->GetSubsystem<USoundClassMixerSubsystem>();
	checkf(SoundClassMixerSubsystem, TEXT("SoundClassMixerSubsystem is invalid."))

	SoundClassMixerSubsystem->SetSoundSubmixVolumeInternal(
		TargetClass, NewVolume
	);
}

float USoundClassMixerBlueprintFunctionLibrary::GetSoundSubmixVolume(USoundSubmix* TargetClass)
{
	if (!TargetClass)
	{
		return -1.f;
	}
	
	return TargetClass->OutputVolume;
}