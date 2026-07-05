#include "SoundClassMixerBlueprintFunctionLibrary.h"
#include "USoundBaseWrapper.h"
#include "Kismet/GameplayStatics.h"


void USoundClassMixerBlueprintFunctionLibrary::PlaySound2D_WithSubmixOverride(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	float VolumeMultiplier, float PitchMultiplier, float StartTime,
	USoundConcurrency* ConcurrencySettings,
	AActor* OwningActor,
	bool bIsUISound,
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
	
	UGameplayStatics::PlaySound2D(
		WorldContextObject,
		Wrapper,
		VolumeMultiplier, PitchMultiplier, StartTime,
		ConcurrencySettings,
		OwningActor,
		bIsUISound
	);
}

void USoundClassMixerBlueprintFunctionLibrary::PlaySoundAtLocation_WithSubmixOverride(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	FVector Location, FRotator Rotation,
	float VolumeMultiplier, float PitchMultiplier, float StartTime,
	USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings,
	AActor* OwningActor,
	USoundSubmixBase* SubmixOverride
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