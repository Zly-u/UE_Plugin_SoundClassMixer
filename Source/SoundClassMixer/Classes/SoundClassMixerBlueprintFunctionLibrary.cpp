#include "SoundClassMixerBlueprintFunctionLibrary.h"

#include "SoundClassMixerSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundSubmix.h"

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

	const FSoundSubSysProperties* FoundSoundClassProps = SoundClassMixerSubsystem->SoundClassMap.Find(TargetClass);
	checkf(FoundSoundClassProps, TEXT("SoundClass Properties are not found."))
	
	SoundClassMixerSubsystem->AdjustSoundClassVolumeInternal(
		TargetClass,
		FadeDuration, FadeVolumeLevel,
		FoundSoundClassProps->Fader.GetVolume() > FadeVolumeLevel,
		FadeCurve
	);
}

void USoundClassMixerBlueprintFunctionLibrary::SetSoundClassVolume(const UObject* WorldContextObject, USoundClass* TargetClass, const float NewVolume)
{
	if (!TargetClass)
	{
		return;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	checkf(World, TEXT("World is invalid."))

	const UGameInstance* GI = World->GetGameInstance();
	checkf(GI, TEXT("GI is invalid."))

	const USoundClassMixerSubsystem* SoundClassMixerSubsystem = GI->GetSubsystem<USoundClassMixerSubsystem>();
	checkf(SoundClassMixerSubsystem, TEXT("SoundClassMixerSubsystem is invalid."))

	SoundClassFadeTo(
		WorldContextObject,
		TargetClass,
		0.f, NewVolume,
		EAudioFaderCurve::Linear
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

// =====================================================================================================================

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

	const FSoundSubSysProperties* FoundSoundClassProps = SoundClassMixerSubsystem->SoundSubmixMap.Find(TargetClass);
	checkf(FoundSoundClassProps, TEXT("SoundClass Properties are not found."))
	
	SoundClassMixerSubsystem->AdjustSoundSubmixVolumeInternal(
		TargetClass,
		FadeDuration, FadeVolumeLevel,
		FoundSoundClassProps->Fader.GetVolume() > FadeVolumeLevel,
		FadeCurve
	);
}

void USoundClassMixerBlueprintFunctionLibrary::SetSoundSubmixVolume(const UObject* WorldContextObject, USoundSubmix* TargetClass, float NewVolume)
{
	if (!TargetClass)
	{
		return;
	}

	SoundSubmixFadeTo(
		WorldContextObject,
		TargetClass,
		0.f, NewVolume,
		EAudioFaderCurve::Linear
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

// =====================================================================================================================

float USoundClassMixerBlueprintFunctionLibrary::ConvertToLinear(float Value)
{
	return Audio::ConvertToLinear(Value);
}

float USoundClassMixerBlueprintFunctionLibrary::ConvertToDecibels(const float Value)
{
	return Audio::ConvertToDecibels(Value);
}
