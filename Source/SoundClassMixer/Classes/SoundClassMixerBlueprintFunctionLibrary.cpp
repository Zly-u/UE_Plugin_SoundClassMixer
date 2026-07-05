#include "SoundClassMixerBlueprintFunctionLibrary.h"

#include "AudioDevice.h"
#include "SoundClassMixerSubsystem.h"
#include "USoundBaseWrapper.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
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


// =====================================================================================================================


void USoundClassMixerBlueprintFunctionLibrary::PlaySound2D_WithSubmixOverride(
	const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier, float StartTime, USoundConcurrency* ConcurrencySettings, AActor* OwningActor, bool bIsUISound,
	USoundSubmixBase* SubmixOverride
)
{
	if (!Sound)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("[USoundClassMixerBlueprintFunctionLibrary::SpawnSoundAttached_WithSubmixOverride] Sound was not set."));
		return;
	}
	
	USoundBaseWrapper* Wrapper = NewObject<USoundBaseWrapper>(GetTransientPackage());
	Wrapper->InheritSoundBase(Sound);
	Wrapper->SoundSubmixOverride = SubmixOverride;
	
	UGameplayStatics::PlaySound2D(WorldContextObject, Wrapper, VolumeMultiplier, PitchMultiplier, StartTime, ConcurrencySettings, OwningActor, bIsUISound);
}

void USoundClassMixerBlueprintFunctionLibrary::PlaySoundAtLocation_WithSubmixOverride(
	const UObject* WorldContextObject, USoundBase* Sound, FVector Location, FRotator Rotation, float VolumeMultiplier,
	float PitchMultiplier, float StartTime, USoundAttenuation* AttenuationSettings,
	USoundConcurrency* ConcurrencySettings, AActor* OwningActor, USoundSubmixBase* SubmixOverride
)
{
	if (!Sound)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("[USoundClassMixerBlueprintFunctionLibrary::PlaySoundAtLocation_WithSubmixOverride] Sound was not set."));
		return;
	}
	
	USoundBaseWrapper* Wrapper = NewObject<USoundBaseWrapper>(GetTransientPackage());
	Wrapper->InheritSoundBase(Sound);
	Wrapper->SoundSubmixOverride = SubmixOverride;
	
	UGameplayStatics::PlaySoundAtLocation(
		WorldContextObject,
		Wrapper,
		Location, Rotation,
		VolumeMultiplier, PitchMultiplier, StartTime,
		AttenuationSettings, ConcurrencySettings,
		OwningActor
	);
}


UAudioComponent* USoundClassMixerBlueprintFunctionLibrary::SpawnSoundAttached_WithSubmixOverride(
	USoundBase* Sound,
	USceneComponent* AttachToComponent, FName AttachPointName,
	FVector Location, FRotator Rotation, EAttachLocation::Type LocationType,
	bool bStopWhenAttachedToDestroyed,
	float VolumeMultiplier, float PitchMultiplier, float StartTime,
	USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings,
	bool bAutoDestroy,
	USoundSubmixBase* SubmixOverride
)
{
	if (!Sound)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("[USoundClassMixerBlueprintFunctionLibrary::SpawnSoundAttached_WithSubmixOverride] Sound was not set."));
		return nullptr;
	}
	
	USoundBaseWrapper* Wrapper = NewObject<USoundBaseWrapper>(GetTransientPackage());
	Wrapper->InheritSoundBase(Sound);
	Wrapper->SoundSubmixOverride = SubmixOverride;
	
	return UGameplayStatics::SpawnSoundAttached(
		Wrapper,
		AttachToComponent, AttachPointName,
		Location, Rotation, LocationType,
		bStopWhenAttachedToDestroyed,
		VolumeMultiplier, PitchMultiplier, StartTime,
		AttenuationSettings, ConcurrencySettings,
		bAutoDestroy
	);
}


UAudioComponent* USoundClassMixerBlueprintFunctionLibrary::SpawnSoundAtLocation_WithSubmixOverride(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	FVector Location, FRotator Rotation,
	float VolumeMultiplier, float PitchMultiplier, float StartTime,
	USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings, bool bAutoDestroy,
	USoundSubmixBase* SubmixOverride
)
{
	if (!Sound)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("[USoundClassMixerBlueprintFunctionLibrary::SpawnSoundAtLocation_WithSubmixOverride] Sound was not set."));
		return nullptr;
	}
	
	USoundBaseWrapper* Wrapper = NewObject<USoundBaseWrapper>(GetTransientPackage());
	Wrapper->InheritSoundBase(Sound);
	Wrapper->SoundSubmixOverride = SubmixOverride;
	
	return UGameplayStatics::SpawnSoundAtLocation(
		WorldContextObject,
		Wrapper,
		Location, Rotation,
		VolumeMultiplier, PitchMultiplier, StartTime,
		AttenuationSettings, ConcurrencySettings, bAutoDestroy
	);
}


UAudioComponent* USoundClassMixerBlueprintFunctionLibrary::SpawnSound2D_WithSubmixOverride(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	float VolumeMultiplier, float PitchMultiplier, float StartTime,
	USoundConcurrency* ConcurrencySettings, bool bPersistAcrossLevelTransition,
	bool bAutoDestroy,
	USoundSubmixBase* SubmixOverride
)
{
	if (!Sound)
	{
		UE_LOG(LogSoundClassMixer, Error, TEXT("[USoundClassMixerBlueprintFunctionLibrary::SpawnSound2D_WithSubmixOverride] Sound was not set."));
		return nullptr;
	}
	
	USoundBaseWrapper* Wrapper = NewObject<USoundBaseWrapper>(GetTransientPackage());
	Wrapper->InheritSoundBase(Sound);
	Wrapper->SoundSubmixOverride = SubmixOverride;
	
	return UGameplayStatics::SpawnSound2D(
		WorldContextObject,
		Wrapper,
		VolumeMultiplier, PitchMultiplier, StartTime,
		ConcurrencySettings,
		bPersistAcrossLevelTransition,
		bAutoDestroy
	);
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
