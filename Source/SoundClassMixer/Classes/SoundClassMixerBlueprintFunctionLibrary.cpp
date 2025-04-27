#include "SoundClassMixerBlueprintFunctionLibrary.h"

#include "SoundClassMixerSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

void USoundClassMixerBlueprintFunctionLibrary::SoundClassFadeTo(
	const UObject* WorldContextObject,
	USoundClass* TargetClass,
	const float FadeOutDuration, const float FadeVolumeLevel,
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

	const FSoundClassSubSysProperties* FoundSoundClassProps = SoundClassMixerSubsystem->SoundClassMap.Find(TargetClass);
	checkf(FoundSoundClassProps, TEXT("SoundClass Properties are not found."))
	
	SoundClassMixerSubsystem->AdjustVolumeInternal(
		TargetClass,
		FadeOutDuration, FadeVolumeLevel,
		FoundSoundClassProps->Fader.GetVolume() > FadeVolumeLevel,
		FadeCurve
	);
}

void USoundClassMixerBlueprintFunctionLibrary::SetSoundClassVolume(USoundClass* TargetClass, const float NewVolume)
{
	if (!TargetClass)
	{
		return;
	}
	
	TargetClass->Properties.Volume = NewVolume;
}

float USoundClassMixerBlueprintFunctionLibrary::GetSoundClassVolume(USoundClass* TargetClass)
{
	if (!TargetClass)
	{
		return -1.f;
	}
	
	return TargetClass->Properties.Volume;
}

float USoundClassMixerBlueprintFunctionLibrary::ConvertToLinear(float Value)
{
	return Audio::ConvertToLinear(Value);
}

float USoundClassMixerBlueprintFunctionLibrary::ConvertToDecibels(const float Value)
{
	return Audio::ConvertToDecibels(Value);
}
