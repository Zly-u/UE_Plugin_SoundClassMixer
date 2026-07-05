#include "SoundClassMixerBlueprintFunctionLibrary.h"

#include "AudioDevice.h"
#include "SoundClassMixerSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"


// =====================================================================================================================


void USoundClassMixerBlueprintFunctionLibrary::SoundClassFadeTo(
	const UObject* WorldContextObject,
	USoundClass* TargetClass,
	const float FadeDuration, const float FadeVolumeLevel,
	const EAudioFaderCurve FadeCurve
)
{
	if (!TargetClass)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("Could not find Sound Class"));
		return;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	checkf(World, TEXT("World is invalid."))

	const UGameInstance* GI = World->GetGameInstance();
	checkf(GI, TEXT("GI is invalid."))
	
	USoundClassMixerSubsystem* SoundClassMixerSubsystem = GI->GetSubsystem<USoundClassMixerSubsystem>();
	checkf(SoundClassMixerSubsystem, TEXT("SoundClassMixerSubsystem is invalid."))

	checkf(SoundClassMixerSubsystem->SoundClassMap.Find(TargetClass), TEXT("SoundClass Properties are not found."))

	SoundClassMixerSubsystem->AdjustSoundClassVolumeInternal(
		TargetClass,
		FadeDuration, FadeVolumeLevel,
		TargetClass->Properties.Volume > FadeVolumeLevel,
		FadeCurve
	);
}

void USoundClassMixerBlueprintFunctionLibrary::SetSoundClassVolume(const UObject* WorldContextObject, USoundClass* TargetClass, const float NewVolume)
{
	if (!TargetClass)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("Could not find Sound Class!"));
		return;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	checkf(World, TEXT("World is invalid."))

	const UGameInstance* GI = World->GetGameInstance();
	checkf(GI, TEXT("GI is invalid."))
	
	USoundClassMixerSubsystem* SoundClassMixerSubsystem = GI->GetSubsystem<USoundClassMixerSubsystem>();
	checkf(SoundClassMixerSubsystem, TEXT("SoundClassMixerSubsystem is invalid."))

	SoundClassMixerSubsystem->SetSoundClassVolumeInternal(
		TargetClass, NewVolume
	);
}

float USoundClassMixerBlueprintFunctionLibrary::GetSoundClassVolume(USoundClass* TargetClass)
{
	if (!TargetClass)
	{
		return -1.f;
	}
	
	return TargetClass->Properties.Volume;
}
